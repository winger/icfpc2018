#pragma once

#include "matrix.h"
#include "problem.h"
#include "trace.h"

class Solver
{
public:
    static uint64_t Solve(const Problem& p, const Matrix& m, Trace& output);
    static unsigned Solve(const Problem& p);
    static CheckResult Check(const Problem& p);

    static Problems ListProblems(const std::string& round);

    static void SolveAll(const std::string& round);
    static void CheckAll(const std::string& round);
    static void MergeWithSubmit(const std::string& round);
};
