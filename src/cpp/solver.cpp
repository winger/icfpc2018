#include "solver.h"

#include <future>
#include <thread>

#include "solvers/layers_base.h"
#include "solvers/layers_parallel.h"

#include "evaluation.h"

#include "command_line.h"
#include "threadPool.h"

void WriteEnergyToFile(uint64_t energy, const string& filename) {
  string full_filename = "../../" + filename;
  ofstream file(full_filename);
  file << energy << endl;
  file.close();
}

uint64_t Solver::Solve(unsigned model_index, const Matrix& m, Trace& output)
{
    Trace temp;
    vector<Trace> traces;
    SolverLayersBase::Solve(m, temp); traces.push_back(temp);
    SolverLayersParallel::Solve(m, temp, false); traces.push_back(temp);
    SolverLayersParallel::Solve(m, temp, true); traces.push_back(temp);
    temp.ReadFromFile("proxyTracesL/LA" + to_string(1000 + model_index).substr(1) + ".nbt"); traces.push_back(temp);
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

unsigned Solver::Solve(unsigned model_index)
{
    string si = to_string(1000 + model_index).substr(1);
    Matrix model;
    model.ReadFromFile("LA" + si + "_tgt");
    Trace trace;
    uint64_t energy = Solve(model_index, model, trace);
    uint64_t energy2 = Evaluation::CheckSolution(model, trace);
    assert((energy == 0) || (energy == energy2));
    WriteEnergyToFile(energy2, "tracesEnergyL/LA" + si + ".txt");

    Trace trace_dflt;
    trace_dflt.ReadFromFile("dfltTracesL/LA" + si + ".nbt");
    uint64_t energy3 = Evaluation::CheckSolution(model, trace_dflt);
    double performance = ((energy2 >= energy3) || (energy2 == 0)) ? 0 : (1.0 - double(energy2) / double(energy3));
    unsigned score = unsigned(1000.0 * performance * unsigned(log(model.GetR()) / log(2)));
    cout << "Test " << si << ": " << performance << endl;
    trace.WriteToFile("cppTracesL/LA" + si + ".nbt");
    return score;
}

static constexpr size_t N_TESTS = 186;

void Solver::SolveAll() {
    unsigned total_score = 0;
    auto threads = cmd.int_args["threads"];
    if (threads > 1) {
        tp::ThreadPoolOptions options;
        options.setThreadCount(threads);
        tp::ThreadPool pool(options);

        std::vector<std::future<unsigned>> futures;
        for (unsigned i = 1; i <= N_TESTS; ++i) {
            std::packaged_task<unsigned()> t([i]() { return Solve(i); });
            futures.emplace_back(t.get_future());
            pool.blockingPost(t);
        }
        for (auto& f : futures) {
            total_score += f.get();
        }
    } else {
        for (unsigned i = 1; i <= N_TESTS; ++i) {
            total_score += Solve(i);
        }
    }
    cout << "Final score: " << total_score << endl;
}
