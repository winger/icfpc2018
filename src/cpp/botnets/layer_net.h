#pragma once

#include "../solver.h"
#include "../state.h"

class LayerNet
{
protected:
    Matrix matrix;
    std::vector<> order;

    SolverGravitated(const Matrix& m);

    void Solve(Trace& output);
    void Order();


public:
  Spread(Trace& output);
  LevelUp(int level, Trace& output);
  CoverPatch(std::vector<int> layer, Trace& output);
  Merge(Trace& output);
  Origin(Trace& output);
  Halt(Trace& output);
};
