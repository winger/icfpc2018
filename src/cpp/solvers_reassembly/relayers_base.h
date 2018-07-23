#pragma once

#include "../evaluation.h"
#include "../solvers_util.h"
#include "../state.h"

class ReassemblySolverLayersBase : public SolverBase {
   protected:
    Matrix source;
    Matrix target;

    ReassemblySolverLayersBase(const Matrix& source, const Matrix& target, bool levitation);

    bool NeedChange(const Coordinate& c) const;
    void Solve(Trace& output);
    size_t GreedyReassemble(size_t& count);
    size_t GreedyFill(const Coordinate& c0, bool dry, size_t& count);

   public:
    static Evaluation::Result Solve(const Matrix& source, const Matrix& target, Trace& output, bool levitation);
};
