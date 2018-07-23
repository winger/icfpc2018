#pragma once

#include "../evaluation.h"
#include "../solver.h"
#include "../state.h"
#include "helpers.h"

class SolverCubeDemolition_Tuned {
protected:
  Matrix matrix;
  State state;

  // int total_bots_in_layer{0};
  vector<CommandGroup> despawn_groups;

  // bounding box
  int x0;
  int x1;
  int y0;
  int y1;
  int z0;
  int z1;

  void SpawnGrid(int xmin, int xmax, int zmin, int zmax);
  void PullUpKurwa();
  void Shrink(int xmin, int xmax, int zmin, int zmax);
  void DemolishCube();

  // copy-copy-pasta
  Coordinate& GetBotPosition() { return state.all_bots[0].c; }
  void AddCommand(const Command& c) { state.trace.commands.push_back(c); state.Step(); }
  void ExecuteCommands(const CommandGroup& group);
  void ExecuteCommandGroups(const vector<CommandGroup>& groups);

  void MoveToCoordinateXZ(int x, int z);

  SolverCubeDemolition_Tuned(const Matrix& m);
  void Solve(Trace& output);

public:
  static Evaluation::Result Solve(const Matrix& target, Trace& output);


};
