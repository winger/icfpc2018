#include "solver.h"

#include <regex>

#include "solvers_assembly/gravitated.h"
#include "solvers_assembly/non_gravitated.h"
#include "solvers_assembly/layers_base.h"
#include "solvers_assembly/layers_parallel.h"
#include "solvers_disassembly/2d_demolition.h"
#include "solvers_disassembly/cube_demolition.h"
#include "solvers_disassembly/cube_demolition_tuned.h"
#include "solvers_reassembly/relayers_base.h"

#include "auto_harmonic.h"
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

static constexpr size_t REASSEMBLE_THRESHOLD = 31;
static constexpr size_t BASE_AND_BOTS_THRESHOLD = 70;

namespace {
void WriteEnergyToFile(uint64_t energy, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << filename << " not found." << endl;
    }
    assert(file.is_open());
    file << energy << endl;
    file.close();
}

void WriteLog(ofstream& file, const Trace& trace, const Evaluation::Result& result) {
  file << "trace=" << trace.tag ;
  file << " energy=" << result.energy;
  file << " correct=" << result.correct << endl;
}
} // namespace

template <typename T>
std::vector<T> runForEachProblem(const std::string& round, std::function<T(const Problem&)> f) {
    auto problems = Solver::ListProblems(round);

    auto threads = cmd.int_args["threads"];
    std::vector<T> result;
    if (threads > 1) {
        cout << "run in " << threads << " threads" << endl;
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
            result = filtered;
        }
    }

    if (cmd.int_args["psort"]) {
        std::sort(result.begin(), result.end(),
                  [](const Problem& a, const Problem& b) -> bool { return a.index < b.index; });
    }

    if (cmd.int_args["prev"]) {
        std::reverse(result.begin(), result.end());
    }

    return result;
}

bool Solver::FindBestTrace(const Problem& p, const Matrix& source, const Matrix& target,
                           const vector<Trace>& traces_to_check, Trace& output, bool write) {
    ofstream file(p.GetLogFile());
    Evaluation::Result best_result;
    for (const auto& trace : traces_to_check) {
        Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
        cout << "trace: " << trace.tag << " --> " << result.energy << " correct: " << result.correct
             << " time: " << trace.Duration() << endl;
        WriteLog(file, trace, result);
        if (result <= best_result) {
            best_result = result;
            output = trace;
        }
    }
    if (best_result.correct && write) {
        // cout << "best trace: " << output.tag << endl;
        // cout << "trace: " << trace.tag << " --> " << result.energy << endl;
        output.WriteToFile(p.GetProxy());
    }
    return best_result.correct;
}

namespace {

using Traces = vector<Trace>;

void ApplyAutoHarmonic(const Matrix& source, const Matrix& target, Traces& tr) {
    if (cmd.int_args["ah"]) {
        size_t old_size = tr.size();
        for (size_t i = 0; i < old_size; ++i) {
            Trace temp;
            AutoHarmonic::ImproveTrace(source, target, tr[i], temp);
            if (temp.commands.size() > 0) {
                tr.emplace_back(std::move(temp));
            }
        }
    }
}

}  // namespace

void Solver::SolveAssemble(const Problem& p, const Matrix& source, const Matrix& target, Trace& output)
{
    Traces traces;

    if (cmd.int_args["base"]) {
        Trace temp;
        AssemblySolverLayersParallel::Solve(source, target, temp, AssemblySolverLayersParallel::base, true);
        temp.tag = "parallel_base";
        traces.push_back(temp);
    }

    if (Grounder::IsByLayerGrounded(target)) {
        try {
          Trace temp;
          SolverGravitated::Solve(target, temp, false);
          temp.tag = "gravitated_solver_stupid";
          traces.push_back(temp);
        } catch (std::runtime_error const& e) {
          cerr << "Error: " << e.what() << endl;
        }

        try {
          Trace temp;
          SolverGravitated::Solve(target, temp, true);
          temp.tag = "gravitated_solver_smart";
          traces.push_back(temp);
        } catch (std::runtime_error const& e) {
          cerr << "Error: " << e.what() << endl;
        }
    }

    try {
      Trace temp;
      SolverNonGravitated::Solve(target, temp, false, false);
      temp.tag = "non_gravitated_solver_stupid";
      traces.push_back(temp);
    } catch (std::runtime_error const& e) {
      cerr << "Error: " << e.what() << endl;
    }

    try {
      Trace temp;
      SolverNonGravitated::Solve(target, temp, true, false);
      temp.tag = "non_gravitated_solver_smart";
      traces.push_back(temp);
    } catch (std::runtime_error const& e) {
      cerr << "Error: " << e.what() << endl;
    }

    try {
      Trace temp;
      SolverNonGravitated::Solve(target, temp, true, true);
      temp.tag = "non_gravitated_solver_smart_naive";
      traces.push_back(temp);
    } catch (std::runtime_error const& e) {
      cerr << "Error: " << e.what() << endl;
    }

    if (cmd.int_args["base"]) {
        if (source.GetR() < BASE_AND_BOTS_THRESHOLD) {
            Trace temp;
            AssemblySolverLayersParallel::Solve(source, target, temp, AssemblySolverLayersParallel::base_and_bots,
                                                true);
            temp.tag = "parallel_base_and_bots";
            traces.push_back(temp);
        }
    }

    if (p.assembly && (source.GetR() < REASSEMBLE_THRESHOLD)) {
        try {
            Trace temp;
            ReassemblySolverLayersBase::Solve(source, target, temp, true);
            traces.push_back(temp);
        } catch (const StopException& e) {
        }
    }

    assert(!traces.empty());
    if (p.assembly) {
        Trace temp;
        if (FileExists(p.GetProxy())) {
            temp.ReadFromFile(p.GetProxy());
            traces.push_back(temp);
        } else {
            cerr << "[WARN] Baseline trace " << p.GetProxy() << " does not exist." << endl;
        }
    }

    ApplyAutoHarmonic(source, target, traces);

    assert(FindBestTrace(p, source, target, traces, output, p.assembly));
}

void Solver::SolveDisassemble(const Problem& p, const Matrix& source, const Matrix& target, Trace& output) {
    vector<Trace> traces;

    if (cmd.int_args["base"]) {
        Trace trace;
        AssemblySolverLayersBase::Solve(source, trace, true, true);
        trace.tag = "base";
        traces.push_back(trace);
        Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
        assert(result.correct);
    }

    if (p.disassembly && (source.GetR() < REASSEMBLE_THRESHOLD)) {
        try {
            Trace temp;
            ReassemblySolverLayersBase::Solve(source, target, temp, true);
            traces.push_back(temp);
        } catch (const StopException& e) {
        }
    }

    if (!cmd.int_args["levitation"]) {
        try {
            Trace trace;
            AssemblySolverLayersBase::Solve(source, trace, true, false);
            trace.tag = "base_no_levitation";
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
            // cout << "Start Evaluation" << endl;
            Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
            assert(result.correct);
        } catch (const StopException& e) {
            // cout << "[WARN] Problem " << p.Name() << " is not supported for 2D demolition" << endl;
        }
    }
    // assert(false);

    try {
        Trace trace;
        SolverCubeDemolition::Solve(source, trace);
        trace.tag = "SolverCubeDemolition";
        traces.push_back(trace);
        // cout << "Start Evaluation" << endl;
        Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
        assert(result.correct);
    } catch (const StopException& e) {
    } catch (const UnsupportedException& e) {
      // cout << "[WARN] Problem " << p.Name() << " is not supported for Cube demolition" << endl;
    }
    // assert(false);

    try {
        Trace trace;
        SolverCubeDemolition_Tuned::Solve(source, trace);
        trace.tag = "SolverCubeDemolition_Tuned";
        traces.push_back(trace);
        // cout << "Start Evaluation" << endl;
        Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
        assert(result.correct);
    } catch (const StopException& e) {
    } catch (const UnsupportedException& e) {
      // cout << "[WARN] Problem " << p.Name() << " is not supported for Cube demolition" << endl;
    }
    // assert(false);

    if (p.disassembly) {
        if (FileExists(p.GetProxy())) {
            Trace temp;
            temp.ReadFromFile(p.GetProxy());
            traces.emplace_back(std::move(temp));
        } else {
            cerr << "[WARN] Baseline trace " << p.GetProxy() << " does not exist." << endl;
        }
    }

    ApplyAutoHarmonic(source, target, traces);

    assert(FindBestTrace(p, source, target, traces, output, p.disassembly));
}

void Solver::SolveReassemble(const Problem& p, const Matrix& source, const Matrix& target, Trace& output) {
    vector<Trace> traces;

    assert(source.GetR() == target.GetR());

    Matrix voidM(source.GetR());

    if (source.GetR() < REASSEMBLE_THRESHOLD) {
        try {
            Trace temp;
            ReassemblySolverLayersBase::Solve(source, target, temp, true);
            traces.push_back(temp);
        } catch (const StopException& e) {
        }
    }

    {
        Trace tmp1;
        SolveDisassemble(p, source, voidM, tmp1);
        Trace tmp2;
        SolveAssemble(p, voidM, target, tmp2);
        traces.emplace_back(Trace::Cat(tmp1, tmp2));
    }

    if (FileExists(p.GetProxy())) {
        Trace temp;
        temp.ReadFromFile(p.GetProxy());
        traces.emplace_back(std::move(temp));
    } else {
        cerr << "[WARN] Baseline trace " << p.GetProxy() << " does not exist." << endl;
    }

    ApplyAutoHarmonic(source, target, traces);
    assert(FindBestTrace(p, source, target, traces, output, true));
}

std::mutex solving_mutex;
std::unordered_set<std::string> solving;

Solution Solver::Solve(const Problem& p) {
    try {
        {
            std::lock_guard<std::mutex> guard(solving_mutex);
            solving.insert(p.Name());
        }

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
        cout << "Test " << p.Name() << ": " << s.score << " " << s.max_score << " r=" << solution_result.r << endl;
        {
            std::lock_guard<std::mutex> guard(solving_mutex);
            solving.erase(p.Name());
        }
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

    Trace trace_dflt;
    trace_dflt.ReadFromFile(p.GetDefaultTrace());
    Evaluation::Result default_result = Evaluation::Evaluate(source, target, trace_dflt);
    Solution s;
    s.trace = trace;
    s.Set(result, default_result);
    cout << p.Name() << " " << ((result.correct) ? "OK" : "Failed") << " " << s.score << " " << s.max_score <<  endl;
    return s;
}

void Solver::SolveAll(const std::string& round) {
    bool stop_watching = false;

    std::thread t_watch([&stop_watching]() {
        while (!stop_watching) {
            {
                std::lock_guard<std::mutex> guard(::solving_mutex);
                std::string s = "Solving: ";
                for (const auto& ss : ::solving) {
                    s += ss + " ";
                }
                cout << s << endl;
            }
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    });

    auto solveResults = runForEachProblem<Solution>(round, [](const Problem& p) { return Solve(p); });

    unsigned total_score = 0;
    unsigned total_max_score = 0;
    for (const auto& s : solveResults) {
        total_score += s.score;
        total_max_score += s.max_score;
    }

    cout << "Final score: " << total_score << " " << total_max_score << endl;

    stop_watching = true;
    t_watch.join();
}

void Solver::CheckAll(const std::string& round) {
    auto checkResults = runForEachProblem<Solution>(round, [](const Problem& p) { return Check(p, p.GetSubmitOutput()); });

    size_t total_ok = 0;
    unsigned total_score = 0;
    unsigned total_max_score = 0;
    for (const auto& cr: checkResults) {
        total_ok += cr.correct;
        total_score += cr.score;
        total_max_score += cr.max_score;
    }

    std::cout << total_ok << "/" << checkResults.size() << " Score to ideal: " << total_max_score - total_score << std::endl;
}

struct MergeResult {
    bool updated;
    int score_diff;
};

MergeResult MergeProblemWithSubmit(const Problem& p) {
    if (!FileExists(p.GetProxy())) {
        cout << p.index << ": NOTHING in src" << endl;
        return {false, 0};
    }

    auto checkProxy = Solver::Check(p, p.GetProxy());
    auto checkSubmit = Solver::Check(p, p.GetSubmitOutput());
    assert(checkProxy.correct);
    assert(checkSubmit.correct);

    bool need_replace = checkProxy < checkSubmit;
    int score_diff = 0;
    if (need_replace) {
        Trace t;
        t.ReadFromFile(p.GetProxy());
        t.WriteToFile(p.GetSubmitOutput());
        WriteEnergyToFile(checkProxy.energy, p.GetSubmitEnergyInfo());
        cout << p.Name() << ": BETTER " << checkProxy.energy << " < " << checkSubmit.energy << endl;
        score_diff = checkProxy.score - checkSubmit.score;
    } else {
        cout << p.Name() << ": NOT BETTER" << endl;
    }
    return {need_replace, score_diff};
}

void Solver::MergeWithSubmit(const std::string& round) {
    auto mergeResults = runForEachProblem<MergeResult>(round, [](const Problem& p) { return MergeProblemWithSubmit(p); });

    size_t total_ok = 0;
    int score_diff = 0;
    for (const auto& cr: mergeResults) {
        total_ok += cr.updated;
        score_diff += cr.score_diff;
    }

    cout << "Merge replaced " << total_ok << " solutions with " << score_diff << " gain." << endl;
}

int WriteMetadataForProblem(const Problem& p) {
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

    Trace trace_dflt;
    trace_dflt.ReadFromFile(p.GetDefaultTrace());
    Evaluation::Result default_result = Evaluation::Evaluate(source, target, trace_dflt);
    auto max_score = unsigned(1000.0 * unsigned(log(default_result.r) / log(2)));

    ofstream file(p.GetMetadataFile());
    file << "dflt_energy=" << default_result.energy << endl;
    file << "R=" << default_result.r << endl;
    file << "max_score=" << max_score << endl;
    cout << p.Name() << ": " << default_result.energy << endl;
    return 0;
}

void Solver::WriteMetadata() {
    auto results = runForEachProblem<int>("full", [](const Problem& p) { return WriteMetadataForProblem(p); });
}
