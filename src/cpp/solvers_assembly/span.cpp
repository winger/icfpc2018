#include "span.h"

#include "../timer.h"
#include "../constants.h"

SolverSpan::SolverSpan(const Matrix& m)
{
    matrix = m;
}

void SolverSpan::Solve(Trace& output)
{
    output.commands.resize(0);

    output.commands.push_back(Command(Command::Halt));
}

uint64_t SolverSpan::Solve(const Matrix& m, Trace& output)
{
    SolverSpan solver(m);
    solver.Solve(output);
    return 0;
}
