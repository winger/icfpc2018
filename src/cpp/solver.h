#pragma once

#include "matrix.h"
#include "trace.h"

class Solver
{
public:
    static uint64_t Solve(const Matrix& m, Trace& output);
    static void Solve(unsigned model_index);
    static void SolveAll();
};
