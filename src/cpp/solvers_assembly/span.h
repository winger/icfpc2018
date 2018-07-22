#pragma once

#include "../solver.h"
#include "../state.h"

class SolverSpan
{
protected:
    Matrix matrix;

    SolverSpan(const Matrix& m);

    void Solve(Trace& output);

public:
    static uint64_t Solve(const Matrix& m, Trace& output);
};
