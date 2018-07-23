#pragma once

#include "../solver.h"
#include "../state.h"

class SolverNonGravitated
{
protected:
    Matrix matrix;
    std::vector<std::vector<int>> order;

    SolverNonGravitated(const Matrix& m);

    void Solve(Trace& output, bool smart, bool naive);
    void Order();


public:
    static uint64_t Solve(const Matrix& m, Trace& output, bool smart, bool naive);
};
