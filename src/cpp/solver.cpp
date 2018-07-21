#include "solver.h"

#include "solvers/layers_base.h"

#include "evaluation.h"

#include "command_line.h"
#include "threadPool.h"

uint64_t Solver::Solve(const Matrix& m, Trace& output)
{
    return SolverLayersBase::Solve(m, output);
}

unsigned Solver::Solve(unsigned model_index)
{
    string si = to_string(1000 + model_index).substr(1);
    Matrix model;
    model.ReadFromFile("LA" + si + "_tgt");
    Trace trace;
    uint64_t energy = Solve(model, trace);
    uint64_t energy2 = Evaluation::CheckSolution(model, trace);
    assert(energy == energy2);
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

void Solver::SolveAll()
{
    tp::ThreadPoolOptions options;
    options.setThreadCount(cmd.int_args["threads"]);
    tp::ThreadPool pool(options);

    unsigned total_score = 0;
    for (unsigned i = 1; i <= N_TESTS; ++i) {
        total_score += Solve(i);
    }
    cout << "Final score: " << total_score << endl;
}
