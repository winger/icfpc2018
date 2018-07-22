#include "solver.h"

#include <regex>

#include "solvers_assembly/layers_base.h"
#include "solvers_assembly/layers_parallel.h"
#include "solvers_disassembly/2d_demolition.h"


#include "base.h"
#include "command_line.h"
#include "evaluation.h"
#include "grounder.h"
#include "problem.h"
#include "solution.h"
#include "pool.h"

namespace {
static constexpr size_t N_LIGHTNING_TESTS = 186;
static constexpr size_t N_FULL_ASSEMBLY_TESTS = 186;
static constexpr size_t N_FULL_DISASSEMBLY_TESTS = 186;
static constexpr size_t N_FULL_REASSEMBLY_TESTS = 115;

void WriteEnergyToFile(uint64_t energy, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << filename << " not found." << endl;
    }
    assert (file.is_open());
    file << energy << endl;
    file.close();
}

template <typename T>
std::vector<T> runForEachProblem(const std::string& round, std::function<T(const Problem&)> f) {
    auto problems = Solver::ListProblems(round);

    auto threads = cmd.int_args["threads"];
    std::vector<T> result;
    if (threads > 1) {
        ThreadPool pool(threads);

        std::vector<std::future<T>> futures;
        for (const auto& p : problems) {
            futures.emplace_back(pool.enqueue(f, p));
        }
        for (auto& f : futures) {
            result.emplace_back(f.get());
        }
    } else {
        for (const auto& p : problems) {
            result.emplace_back(f(p));
        }
    }

    return result;
}
}  // namespace

Problems Solver::ListProblems(const std::string& round) {
    Problems result;
    if (round == "lightning") {
        for (size_t i = 1; i <= N_LIGHTNING_TESTS; ++i) {
            Problem p;
            p.index = i;
            p.assembly = true;
            p.round = "L";
            result.emplace_back(std::move(p));
        }
    } else if (round == "full") {
        Problem p0;
        p0.round = "F";
        for (size_t i = 1; i <= N_FULL_ASSEMBLY_TESTS; ++i) {
            Problem p = p0;
            p.index = i;
            p.assembly = true;
            result.emplace_back(std::move(p));
        }
        for (size_t i = 1; i <= N_FULL_DISASSEMBLY_TESTS; ++i) {
            Problem p = p0;
            p.index = i;
            p.disassembly = true;
            result.emplace_back(std::move(p));
        }
        for (size_t i = 1; i <= N_FULL_REASSEMBLY_TESTS; ++i) {
            Problem p = p0;
            p.index = i;
            p.reassembly = true;
            result.emplace_back(std::move(p));
        }
    } else {
        assert(false);
    }

    if (cmd.args.count("test")) {
        const std::regex r(cmd.args["test"]);
        Problems filtered;
        for (const auto& p : result) {
            if (std::regex_match(p.Name(), r)) {
                filtered.emplace_back(p);
            }
        }
        if (!filtered.empty()) {
            return filtered;
        }
    }

    return result;
}

bool Solver::FindBestTrace(const Problem& p, const Matrix& source, const Matrix& target, const vector<Trace>& traces_to_check, Trace& output, bool write) {
    Evaluation::Result best_result;
    for (const Trace& trace : traces_to_check)
    {
        Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
        // cout << "trace: " << trace.tag << " --> " << result.energy << endl;
        if (result < best_result)
        {
            best_result = result;
            output = trace;
        }
    }
    if (best_result.correct && write) {
        output.WriteToFile(p.GetProxy());
    }
    return best_result.correct;
}

void Solver::SolveAssemble(const Problem& p, const Matrix& source, const Matrix& target, Trace& output)
{
    Trace temp;
    vector<Trace> traces;
    AssemblySolverLayersBase::Solve(target, temp, false, true);
    traces.push_back(temp);
    AssemblySolverLayersParallel::Solve(target, temp, AssemblySolverLayersParallel::base, true);
    traces.push_back(temp);
    if (!cmd.int_args["levitation"]) {
        try {
            AssemblySolverLayersBase::Solve(target, temp, false, false);
            traces.push_back(temp);
        } catch (const StopException& e) {
        }
        try {
            AssemblySolverLayersParallel::Solve(target, temp, AssemblySolverLayersParallel::base, false);
            traces.push_back(temp);
        } catch (const StopException& e) {
        }
    }

    assert(!traces.empty());
    if (p.assembly) {
        if (FileExists(p.GetProxy())) {
            temp.ReadFromFile(p.GetProxy());
            traces.push_back(temp);
        } else {
            cerr << "[WARN] Baseline trace " << p.GetProxy() << " does not exist." << endl;
        }
    }
    assert(FindBestTrace(p, source, target, traces, output, p.assembly));
}

void Solver::SolveDisassemble(const Problem& p, const Matrix& source, const Matrix& target, Trace& output) {
    vector<Trace> traces;

    {
        Trace trace;
        AssemblySolverLayersBase::Solve(source, trace, true, true);
        traces.push_back(trace);
        Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
        assert(result.correct);
    }

    if (!cmd.int_args["levitation"]) {
        try {
            Trace trace;
            AssemblySolverLayersBase::Solve(source, trace, true, false);
            traces.push_back(trace);
            Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
            assert(result.correct);
        } catch (const StopException& e) {
        }
    }
    {
        try {
          Trace trace;
          Solver2D_Demolition::Solve(source, trace);
          trace.tag = "Solver2D_Demolition";
          traces.push_back(trace);
          Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
          assert(result.correct);
        } catch (const StopException& e) {
          cout << "[WARN] Problem " << p.Name() << " is not supported for 2D demolition" << endl;
        }
    }

    if (p.disassembly) {
        if (FileExists(p.GetProxy())) {
            Trace temp;
            temp.ReadFromFile(p.GetProxy());
            traces.emplace_back(std::move(temp));
        } else {
            cerr << "[WARN] Baseline trace " << p.GetProxy() << " does not exist." << endl;
        }
    }

    assert(FindBestTrace(p, source, target, traces, output, p.disassembly));
}

void Solver::SolveReassemble(const Problem& p, const Matrix& source, const Matrix& target, Trace& output) {
    vector<Trace> traces;

    assert(source.GetR() == target.GetR());

    Matrix voidM(source.GetR());

    {
        Trace tmp1;
        SolveDisassemble(p, source, voidM, tmp1);
        Trace tmp2;
        SolveAssemble(p, voidM, target, tmp2);
        traces.emplace_back(Trace::Cat(tmp1, tmp2));
    }

    {
        Trace temp;
        temp.ReadFromFile(p.GetDefaultTrace());
        traces.emplace_back(std::move(temp));
    }

    if (FileExists(p.GetProxy())) {
        Trace temp;
        temp.ReadFromFile(p.GetProxy());
        traces.emplace_back(std::move(temp));
    } else {
        cerr << "[WARN] Baseline trace " << p.GetProxy() << " does not exist." << endl;
    }

    assert(FindBestTrace(p, source, target, traces, output, true));
}

Solution Solver::Solve(const Problem& p) {
    try {
        Solution s;
        Matrix source, target;
        if (p.assembly) {
            target.ReadFromFile(p.GetTarget());
            source.Init(target.GetR());
            SolveAssemble(p, source, target, s.trace);
        } else if (p.disassembly) {
            source.ReadFromFile(p.GetSource());
            target.Init(source.GetR());
            SolveDisassemble(p, source, target, s.trace);
        } else if (p.reassembly) {
            source.ReadFromFile(p.GetSource());
            target.ReadFromFile(p.GetTarget());
            SolveReassemble(p, source, target, s.trace);
        }
        s.trace.WriteToFile(p.GetOutput());
        Evaluation::Result solution_result = Evaluation::Evaluate(source, target, s.trace);
        Trace trace_dflt;
        trace_dflt.ReadFromFile(p.GetDefaultTrace());
        Evaluation::Result default_result = Evaluation::Evaluate(source, target, trace_dflt);
        s.Set(solution_result, default_result);
        cout << "Test " << p.Name() << ": " << s.score << " " << s.max_score << endl;
        return s;
    } catch (...) {
        cerr << "Exception in handling '" << p.Name() << "'" << endl;
        throw;
    }
}

Solution Solver::Check(const Problem& p, const std::string& filename) {
    Matrix source, target;
    if (p.assembly) {
        target.ReadFromFile(p.GetTarget());
        source.Init(target.GetR());
    } else if (p.disassembly) {
        source.ReadFromFile(p.GetSource());
        target.Init(source.GetR());
    } else if (p.reassembly) {
        source.ReadFromFile(p.GetSource());
        target.ReadFromFile(p.GetTarget());
    }
    Trace trace;
    trace.ReadFromFile(filename);
    Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
    cout << p.Name() << " " << ((result.correct) ? "OK" : "Failed") << endl;

    Trace trace_dflt;
    trace_dflt.ReadFromFile(p.GetDefaultTrace());
    Evaluation::Result default_result = Evaluation::Evaluate(source, target, trace_dflt);
    Solution s;
    s.trace = trace;
    s.Set(result, default_result);
    return s;
}

void Solver::SolveAll(const std::string& round) {
    auto solveResults = runForEachProblem<Solution>(round, [](const Problem& p) { return Solve(p); });

    unsigned total_score = 0;
    unsigned total_max_score = 0;
    for (const auto& s: solveResults) {
        total_score += s.score;
        total_max_score += s.max_score;
    }

    cout << "Final score: " << total_score << " "  << total_max_score << endl;
}

void Solver::CheckAll(const std::string& round) {
    auto checkResults = runForEachProblem<Solution>(round, [](const Problem& p) { return Check(p, p.GetSubmitOutput()); });

    size_t total_ok = 0;
    unsigned total_score = 0;
    for (const auto& cr: checkResults) {
        total_ok += cr.correct;
        total_score += cr.score;
    }

    std::cout << total_ok << "/" << checkResults.size() << " Score: " << total_score << std::endl;
}

bool MergeProblemWithSubmit(const Problem& p) {
    if (!FileExists(p.GetProxy())) {
        cout << p.index << ": NOTHING in src" << endl;
        return false;
    }

    auto checkProxy = Solver::Check(p, p.GetProxy());
    auto checkSubmit = Solver::Check(p, p.GetSubmitOutput());
    assert(checkProxy.correct);
    assert(checkSubmit.correct);

    bool need_replace = checkProxy < checkSubmit;
    if (need_replace) {
        Trace t;
        t.ReadFromFile(p.GetProxy());
        t.WriteToFile(p.GetSubmitOutput());
        WriteEnergyToFile(checkProxy.energy, p.GetSubmitEnergyInfo());
        cout << p.Name() << ": BETTER " << checkProxy.energy << " < " << checkSubmit.energy << endl;
    } else {
        cout << p.Name() << ": NOT BETTER" << endl;
    }
    return need_replace;
}

void Solver::MergeWithSubmit(const std::string& round) {
    auto mergeResults = runForEachProblem<bool>(round, [](const Problem& p) { return MergeProblemWithSubmit(p); });

    size_t total_ok = 0;
    for (const auto& cr: mergeResults) {
        total_ok += cr;
    }

    cout << "Merge replaced " << total_ok << " solutions." << endl;
}
