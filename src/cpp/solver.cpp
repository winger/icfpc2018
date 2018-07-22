#include "solver.h"

#include "solvers_assembly/layers_base.h"
#include "solvers_assembly/layers_parallel.h"

#include "base.h"
#include "command_line.h"
#include "evaluation.h"
#include "grounder.h"
#include "problem.h"
#include "solution.h"
#include "threadPool.h"

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
}

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
    return result;
}

void Solver::FindBestTrace(const Problem& p, const Matrix& source, const Matrix& target, const vector<Trace>& traces_to_check, Trace& output)
{
    Evaluation::Result best_result;
    for (const Trace& trace : traces_to_check)
    {
        Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
        if (result < best_result)
        {
            best_result = result;
            output = trace;
        }
    }
    if (best_result.correct)
        output.WriteToFile(p.GetProxy());
}

void Solver::SolveAssemble(const Problem& p, const Matrix& source, const Matrix& target, Trace& output)
{
    Trace temp;
    vector<Trace> traces;
    AssemblySolverLayersBase::Solve(target, temp); traces.push_back(temp);
    // try {
    //     SolverLayersParallel::Solve(target, temp, false);
    //     traces.push_back(temp);
    // } catch (const StopException& e) {
    // }
    // try {
    //     SolverLayersParallel::Solve(target, temp, true);
    //     traces.push_back(temp);
    // } catch (const StopException& e) {
    // }
    assert(!traces.empty());
    if (FileExists(p.GetProxy())) {
        temp.ReadFromFile(p.GetProxy()); traces.push_back(temp);
    } else {
        cerr << "[WARN] Baseline trace " << p.GetProxy() << " does not exist." << endl;
    }
    FindBestTrace(p, source, target, traces, output);
}

void Solver::SolveDisassemble(const Problem& p, const Matrix& source, const Matrix& target, Trace& output) {
    Trace temp;
    vector<Trace> traces;

    temp.ReadFromFile(p.GetDefaultTrace());
    traces.push_back(temp);

    if (FileExists(p.GetProxy())) {
        temp.ReadFromFile(p.GetProxy());
        traces.push_back(temp);
    } else {
        cerr << "[WARN] Baseline trace " << p.GetProxy() << " does not exist." << endl;
    }

    FindBestTrace(p, source, target, traces, output);
}

void Solver::SolveReassemble(const Problem& p, const Matrix& source, const Matrix& target, Trace& output) {
    Trace temp;
    vector<Trace> traces;

    temp.ReadFromFile(p.GetDefaultTrace());
    traces.push_back(temp);

    if (FileExists(p.GetProxy())) {
        temp.ReadFromFile(p.GetProxy());
        traces.push_back(temp);
    } else {
        cerr << "[WARN] Baseline trace " << p.GetProxy() << " does not exist." << endl;
    }

    FindBestTrace(p, source, target, traces, output);
}

double Performance(uint64_t energy2, uint64_t energy3) {
    return ((energy2 >= energy3) || (energy2 == 0)) ? 0 : (1.0 - double(energy2) / double(energy3));
}

unsigned Score(const Matrix& model, double performance) {
    return unsigned(1000.0 * performance * unsigned(log(model.GetR()) / log(2)));
}

Solution Solver::Solve(const Problem& p) {
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
    cout << "Test " << p.index << " " << p.GetType() << ": " << s.score << " " << s.max_score << endl;
    return s;
}

Solution Solver::Check(const Problem& p) {
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
    trace.ReadFromFile(p.GetProxy());
    Evaluation::Result result = Evaluation::Evaluate(source, target, trace);
    cout << p.index << " " << ((result.correct) ? "OK" : "Failed") << endl;

    Trace trace_dflt;
    trace_dflt.ReadFromFile(p.GetDefaultTrace());
    Evaluation::Result default_result = Evaluation::Evaluate(source, target, trace_dflt);
    Solution s;
    s.trace = trace;
    s.Set(result, default_result);
    return s;
}

void Solver::SolveAll(const std::string& round) {
    auto problems = ListProblems(round);

    unsigned total_score = 0;
    unsigned total_max_score = 0;
    auto threads = cmd.int_args["threads"];
    if (threads > 1) {
        tp::ThreadPoolOptions options;
        options.setThreadCount(threads);
        tp::ThreadPool pool(options);

        std::vector<std::future<Solution>> futures;
        for (const auto& p: problems) {
            std::packaged_task<Solution()> t([p]() { return Solve(p); });
            futures.emplace_back(t.get_future());
            pool.blockingPost(t);
        }
        for (auto& f : futures) {
            const auto& s = f.get();
            total_score += s.score;
            total_max_score += s.max_score;
        }
    } else {
        for (const auto& p: problems) {
            auto s = Solve(p);
            total_score += s.score;
            total_max_score += s.max_score;
        }
    }
    cout << "Final score: " << total_score << " "  << total_max_score << endl;
}

void Solver::CheckAll(const std::string& round) {
    auto problems = ListProblems(round);

    auto threads = cmd.int_args["threads"];
    std::vector<Solution> total;
    if (threads > 1) {
        tp::ThreadPoolOptions options;
        options.setThreadCount(threads);
        tp::ThreadPool pool(options);

        std::vector<std::future<Solution>> futures;
        for (const auto& p: problems) {
            std::packaged_task<Solution()> t([p]() { return Check(p); });
            futures.emplace_back(t.get_future());
            pool.blockingPost(t);
        }
        for (auto& f : futures) {
            total.emplace_back(f.get());
        }
    } else {
        for (const auto& p: problems) {
            total.emplace_back(Check(p));
        }
    }

    size_t total_ok = 0;
    unsigned total_score = 0;
    for (const auto& cr : total) {
        total_ok += cr.correct;
        total_score += cr.score;
    }

    std::cout << total_ok << "/" << problems.size() << " Score: " << total_score << std::endl;
}

// bool MergeProblemWithSubmit(const Problem& p) {
//     Trace trace;
//     bool ok = trace.TryReadFromFile(p.GetOutput());
//     if (!ok) {
//         cout << p.index << ": NOTHING in src" << endl;
//         return false;
//     }

//     bool need_replace = false;
//     uint64_t energy = 2000000000000ULL;
//     uint64_t energy2 = 0;
//     if (p.assembly) {
//         Matrix model;
//         model.ReadFromFile(p.GetTarget());

//         energy = Evaluation::CheckSolution(model, trace);
//         bool result = energy > 0;

//         Trace traceBest;
//         bool ok2 = traceBest.TryReadFromFile(p.GetSubmitOutput());
//         if (ok2) {
//             energy2 = Evaluation::CheckSolution(model, traceBest);
//             bool result2 = energy2 > 0;
//             if (energy < energy2) {
//                 need_replace = true;
//             }
//         } else {
//             need_replace = true;
//         }

//     } else {
//         cerr << "[ERROR] Merge unsupported modes" << endl;
//         need_replace = true;
//     }
//     if (need_replace) {
//         trace.WriteToFile(p.GetSubmitOutput());
//         WriteEnergyToFile(energy, p.GetSubmitEnergyInfo());
//         cout << p.index << ": BETTER " << energy << " < " << energy2 << endl;
//     } else {
//         cout << p.index << ": NOT BETTER" << endl;
//     }
//     return need_replace;
// }

// void Solver::MergeWithSubmit(const std::string& round) {
//     auto problems = ListProblems(round);
//     unsigned total_score = 0;
//     auto threads = cmd.int_args["threads"];
//     if (threads > 1) {
//         tp::ThreadPoolOptions options;
//         options.setThreadCount(threads);
//         tp::ThreadPool pool(options);

//         std::vector<std::future<unsigned>> futures;
//         for (const auto& p : problems) {
//             std::packaged_task<unsigned()> t([p]() { return MergeProblemWithSubmit(p); });
//             futures.emplace_back(t.get_future());
//             pool.blockingPost(t);
//         }
//         for (auto& f : futures) {
//             total_score += f.get();
//         }
//     } else {
//         for (const auto& p : problems) {
//             total_score = MergeProblemWithSubmit(p);
//         }
//     }
//     cout << "Merge replaced " << total_score << " solutions." << endl;
// }
