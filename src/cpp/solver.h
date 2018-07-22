#pragma once

#include "matrix.h"
#include "evaluation.h"
#include "problem.h"
#include "solution.h"
#include "trace.h"

class Solver
{
public:
    static void FindBestTrace(const Problem& p, const Matrix& source, const Matrix& target, const vector<Trace>& traces_to_check, Trace& output);
    static void SolveAssemble(const Problem& p, const Matrix& source, const Matrix& target, Trace& output);
    static void SolveDisassemble(const Problem& p, const Matrix& source, const Matrix& target, Trace& output);
    static void SolveReassemble(const Problem& p, const Matrix& source, const Matrix& target, Trace& output);
    static Solution Solve(const Problem& p);
    static Solution Check(const Problem& p);

    static Problems ListProblems(const std::string& round);

    static void SolveAll(const std::string& round);
    static void CheckAll(const std::string& round);
    // static void MergeWithSubmit(const std::string& round);
};
