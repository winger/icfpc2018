#pragma once

#include "../solver.h"
#include "../state.h"

class SolverGravitated
{
protected:
    Matrix matrix;
    std::vector<std::vector<int>> order;

    SolverGravitated(const Matrix& m);

    void Solve(Trace& output);
    void Order();


public:
    static uint64_t Solve(const Matrix& m, Trace& output);
};
