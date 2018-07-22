#pragma once

#include "../evaluation.h"
#include "../solver.h"
#include "../state.h"

class Solver2D_Demolition {
protected:
  Matrix matrix;
  State state;

  Solver2Solver2D_Demolition(const Matrix& m);
  void Solve(Trace& output);

public:

  static Evaluation::Result Solve(const Matrix& target, Trace& output);
};
