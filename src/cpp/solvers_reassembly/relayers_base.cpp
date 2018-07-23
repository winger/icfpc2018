#include "relayers_base.h"

ReassemblySolverLayersBase::ReassemblySolverLayersBase(const Matrix& s, const Matrix& t, bool levitation)
    : SolverBase(s, levitation), source(s), target(t) {}

bool ReassemblySolverLayersBase::NeedChange(const Coordinate& c) const {
    return state.matrix.Get(c) != target.Get(c);
}

void ReassemblySolverLayersBase::Solve(Trace& output) {
    SolveInit();

    output.tag = "reassembly";

    throw StopException();

    SolveFinalize();
    output = state.trace;
}

Evaluation::Result ReassemblySolverLayersBase::Solve(const Matrix& source, const Matrix& target, Trace& output, bool levitation) {
    ReassemblySolverLayersBase solver(source, target, levitation);
    solver.Solve(output);
    return Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);

}
