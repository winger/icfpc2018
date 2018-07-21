#pragma once

#include <vector>

#include "matrix.h"
#include "trace.h"

struct CheckResult {
    bool ok;
    unsigned score;
};

struct Problem {
    size_t index;
    std::string si;
    std::string round;
    bool assembly{};
    bool disassembly{};
    bool reassembly{};
    std::string GetType() const;
    std::string GetSI() const;
    std::string GetTarget() const;
    std::string GetProxy() const;
    std::string GetDefaultTrace() const;
    std::string GetTrace() const;
    std::string GetOutput() const;
};
using Problems = std::vector<Problem>;

class Solver
{
public:
    static uint64_t Solve(const Problem& p, const Matrix& m, Trace& output);
    static unsigned Solve(const Problem& p);
    static CheckResult Check(const Problem& p);

    static Problems ListProblems(const std::string& round);

    static void SolveAll(const std::string& round);
    static void CheckAll(const std::string& round);
};
