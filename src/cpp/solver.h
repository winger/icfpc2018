#pragma once

#include "matrix.h"
#include "trace.h"

struct CheckResult {
    bool ok;
    unsigned score;
};

class Solver
{
public:
    static uint64_t Solve(unsigned model_index, const Matrix& m, Trace& output);
    static unsigned Solve(unsigned model_index);
    static void SolveAll();
    static CheckResult Check(unsigned model_index);
    static void CheckAll();
};
