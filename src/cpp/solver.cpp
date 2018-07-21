#include "solver.h"

#include <future>
#include <thread>

#include "solvers/layers_base.h"
#include "solvers/layers_parallel.h"

#include "evaluation.h"

#include "command_line.h"
#include "threadPool.h"

namespace {
static constexpr size_t N_LIGHTNING_TESTS = 186;
static constexpr size_t N_FULL_ASSEMBLY_TESTS = 186;

void WriteEnergyToFile(uint64_t energy, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << filename << " not found." << endl;
    }
    assert (file.is_open());
    file << energy << endl;
    file.close();
}

std::string Problem::GetType() const {
    assert(assembly + disassembly + reassembly == 1);
    if (assembly) {
        return "A";
    }
    if (reassembly) {
        return "D";
    }
    if (reassembly) {
        return "R";
    }
    assert(false);
}

std::string Problem::GetSI() const { return to_string(1000 + index).substr(1); }

std::string Problem::GetPrefix() const { return "../../"; }

std::string Problem::GetTarget() const {
    auto filename = round + GetType() + GetSI() + "_tgt";

    return GetPrefix() + "problems" + round + "/" + filename + ".mdl";
}

std::string Problem::GetProxy() const { return GetPrefix() + "proxyTraces" + round + "/" + round + GetType() + GetSI() + ".nbt"; }

std::string Problem::GetDefaultTrace() const { return GetPrefix() + "dfltTraces" + round + "/" + round + GetType() + GetSI() + ".nbt"; }

std::string Problem::GetEnergyInfo() const { return GetPrefix() + "tracesEnergy" + round + "/" + round + GetType() + GetSI() + ".txt"; }

std::string Problem::GetOutput() const { return GetPrefix() + "cppTraces" + round + "/" + round + GetType() + GetSI() + ".nbt"; }

std::string Problem::GetSubmitEnergyInfo() const {
    return "submitEnergy" + round + "/" + round + GetType() + GetSI() + ".txt";
}
std::string Problem::GetSubmitOutput() const {
    return "submitTraces" + round + "/" + round + GetType() + GetSI() + ".nbt";
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
        for (size_t i = 1; i <= N_FULL_ASSEMBLY_TESTS; ++i) {
            Problem p;
            p.index = i;
            p.assembly = true;
            p.round = "F";
            result.emplace_back(std::move(p));
        }
    } else {
        assert(false);
    }
    return result;
}

uint64_t Solver::Solve(const Problem& p, const Matrix& m, Trace& output)
{
    Trace temp;
    vector<Trace> traces;
    SolverLayersBase::Solve(m, temp); traces.push_back(temp);
    try {
        SolverLayersParallel::Solve(m, temp, false);
        traces.push_back(temp);
    } catch (const StopException& e) {
    }
    try {
        SolverLayersParallel::Solve(m, temp, true);
        traces.push_back(temp);
    } catch (const StopException& e) {
    }
    assert(!traces.empty());
    if (FileExists(p.GetProxy())) {
        temp.ReadFromFile(p.GetProxy()); traces.push_back(temp);
    } else {
        cerr << "[WARN] Baseline trace " << p.GetProxy() << " does not exists." << endl;
    }
    uint64_t best_energy = -uint64_t(1);
    for (const Trace& trace : traces)
    {
        uint64_t energy = Evaluation::CheckSolution(m, trace);
        if (energy && (best_energy > energy))
        {
            best_energy = energy;
            output = trace;
        }
    }
    return best_energy;
}

double Performance(uint64_t energy2, uint64_t energy3) {
    return ((energy2 >= energy3) || (energy2 == 0)) ? 0 : (1.0 - double(energy2) / double(energy3));
}

unsigned Score(const Matrix& model, double performance) {
    return unsigned(1000.0 * performance * unsigned(log(model.GetR()) / log(2)));
}

unsigned Solver::Solve(const Problem& p)
{
    Matrix model;
    model.ReadFromFile(p.GetTarget());
    Trace trace;
    uint64_t energy = Solve(p, model, trace);
    uint64_t energy2 = Evaluation::CheckSolution(model, trace);
    assert((energy == 0) || (energy == energy2));
    WriteEnergyToFile(energy2, p.GetEnergyInfo());

    Trace trace_dflt;
    trace_dflt.ReadFromFile(p.GetDefaultTrace());
    uint64_t energy3 = Evaluation::CheckSolution(model, trace_dflt);
    auto performance = Performance(energy2, energy3);
    cout << "Test " << p.index << ": " << performance << endl;
    trace.WriteToFile(p.GetOutput());
    auto score = Score(model, performance);
    return score;
}

CheckResult Solver::Check(const Problem& p) {
    Matrix model;
    model.ReadFromFile(p.GetTarget());
    Trace trace;
    trace.ReadFromFile(p.GetProxy());
    auto energy2 = Evaluation::CheckSolution(model, trace);
    bool result = energy2 > 0;
    cout << p.index << " " << ((result) ? "OK" : "Failed") << endl;

    Trace trace_dflt;
    trace_dflt.ReadFromFile(p.GetDefaultTrace());
    uint64_t energy3 = Evaluation::CheckSolution(model, trace_dflt);

    CheckResult check_result;
    check_result.ok = result;
    check_result.score = Score(model, Performance(energy2, energy3));

    return check_result;
}

void Solver::SolveAll(const std::string& round) {
    auto problems = ListProblems(round);

    unsigned total_score = 0;
    auto threads = cmd.int_args["threads"];
    if (threads > 1) {
        tp::ThreadPoolOptions options;
        options.setThreadCount(threads);
        tp::ThreadPool pool(options);

        std::vector<std::future<unsigned>> futures;
        for (const auto& p: problems) {
            std::packaged_task<unsigned()> t([p]() { return Solve(p); });
            futures.emplace_back(t.get_future());
            pool.blockingPost(t);
        }
        for (auto& f : futures) {
            total_score += f.get();
        }
    } else {
        for (const auto& p: problems) {
            total_score += Solve(p);
        }
    }
    cout << "Final score: " << total_score << endl;
}

void Solver::CheckAll(const std::string& round) {
    auto problems = ListProblems(round);

    auto threads = cmd.int_args["threads"];
    std::vector<CheckResult> total;
    if (threads > 1) {
        tp::ThreadPoolOptions options;
        options.setThreadCount(threads);
        tp::ThreadPool pool(options);

        std::vector<std::future<CheckResult>> futures;
        for (const auto& p: problems) {
            std::packaged_task<CheckResult()> t([p]() { return Check(p); });
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
        total_ok += cr.ok;
        total_score += cr.score;
    }

    std::cout << total_ok << "/" << problems.size() << " Score: " << total_score << std::endl;
}

void MergeProblemWithSubmit(const Problem& p) {
  Matrix model;
  model.ReadFromFile(p.GetTarget());
  Trace trace;
  bool ok = trace.TryReadFromFile(p.GetOutput());
  if (!ok) {
    cout << p.index << ": NOTHING in src" << endl;
    return;
  }

  auto energy = Evaluation::CheckSolution(model, trace);
  bool result = energy > 0;

  bool need_replace = false;
  Trace traceBest;
  bool ok2 = traceBest.TryReadFromFile(p.GetSubmitOutput());
  uint64_t energy2 = 0;
  if (ok2) {
    energy2 = Evaluation::CheckSolution(model, traceBest);
    bool result2 = energy2 > 0;
    if (energy < energy2) {
      need_replace = true;
    }
  } else {
    need_replace = true;
  }

  if (need_replace) {
    trace.WriteToFile(p.GetSubmitOutput());
    WriteEnergyToFile(energy, p.GetSubmitEnergyInfo());
    cout << p.index << ": BETTER " << energy << " < " << energy2 << endl;
  } else {
    cout << p.index << ": NOT BETTER" << endl;
  }
}

void Solver::MergeWithSubmit(const std::string& round) {
  auto problems = ListProblems(round);
  for (const auto& p: problems) {
    MergeProblemWithSubmit(p);
  }
}
