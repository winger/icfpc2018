#include "solver.h"

#include "solvers/layers_base.h"

#include "evaluation.h"

uint64_t Solver::Solve(const Matrix& m, Trace& output)
{ 
    return SolverLayersBase::Solve(m, output);
}

void Solver::Solve(unsigned model_index)
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
    cout << "Test " << si << ": " << performance << endl;
}

void Solver::SolveAll()
{
    for (unsigned i = 1; i <= 186; ++i)
        Solve(i);
}
