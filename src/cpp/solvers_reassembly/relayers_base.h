#pragma once

#include "../evaluation.h"
#include "../solvers_util.h"
#include "../state.h"

class ReassemblySolverLayersBase : public SolverBase {
   private:
    Matrix source;
    Matrix target;
    State state;

   public:
    static Evaluation::Result Solve(const Matrix& source, const Matrix& target, Trace& output, bool levitation);
};
