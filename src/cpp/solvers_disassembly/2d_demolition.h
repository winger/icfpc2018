#pragma once

#include "../evaluation.h"
#include "../solver.h"
#include "../state.h"

class Solver2D_Demolition {
protected:
  Matrix matrix;
  State state;

  void AddCommand(const Command& c) { state.trace.commands.push_back(c); state.Step(); }
  Coordinate& GetBotPosition() { return state.all_bots[0].c; }

  void MoveToCoordinate(int x, int z);
  void MoveToCoordinate(int x, int y, int z);
  void SpawnBotsInGrid(int x0, int x1, int z0, int z1);

  Solver2D_Demolition(const Matrix& m);
  void Solve(Trace& output);

public:

  static Evaluation::Result Solve(const Matrix& target, Trace& output);
  static void TestSomething();
};
