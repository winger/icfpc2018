#pragma once

#include "matrix.h"
#include "trace.h"

class Solver
{
public:
    static uint64_t Solve(unsigned model_index, const Matrix& m, Trace& output);
    static unsigned Solve(unsigned model_index);
    static void SolveAll();
    static bool Check(unsigned model_index);
    static void CheckAll();
};
