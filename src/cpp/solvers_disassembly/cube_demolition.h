#pragma once

#include "../evaluation.h"
#include "../solver.h"
#include "../state.h"
#include "helpers.h"

class SolverCubeDemolition {
protected:
  Matrix matrix;
  State state;

  vector<CommandGroup> despawn_groups;

  // bounding box
  int x0;
  int x1;
  int y0;
  int y1;
  int z0;
  int z1;

  void SpawnDoubleBotsOnFloor(int xmin, int xmax, int zmin, int zmax);
  void ReplicateToTop();
  void ShrinkToBottom();

  void DemolishCube();

  SolverCubeDemolition(const Matrix& m);
  void Solve(Trace& output);

public:
  static Evaluation::Result Solve(const Matrix& target, Trace& output);


};
