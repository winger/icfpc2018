#pragma once

#include "matrix.h"
#include "problem.h"
#include "trace.h"

class Solver
{
public:
    static uint64_t SolveAssemble(const Problem& p, const Matrix& m, Trace& output);
    static uint64_t SolveDisassemble(const Problem& p, const Matrix& m, Trace& output);
    static uint64_t SolveReassemble(const Problem& p, const Matrix& src, const Matrix& trg, Trace& output);
    static unsigned Solve(const Problem& p);
    static CheckResult Check(const Problem& p);

    static Problems ListProblems(const std::string& round);

    static void SolveAll(const std::string& round);
    static void CheckAll(const std::string& round);
    static void MergeWithSubmit(const std::string& round);
};
