#pragma once

#include "../evaluation.h"
#include "../solver.h"
#include "../state.h"
#include "helpers.h"

class Solver2D_Demolition {
protected:
  Matrix matrix;
  State state;

  // coordinates of grid how we spawn
  vector<int> x_coords;
  vector<int> z_coords;
  int total_bots_in_layer{0};

  vector<CommandGroup> despawn_groups;

  // indexes compared to position of our bots
  // first is the row with smaller x
  vector<int> first_row;
  vector<int> second_row;

  void AddCommand(const Command& c) { state.trace.commands.push_back(c); state.Step(); }
  void ExecuteCommands(const CommandGroup& group);
  void ExecuteCommandGroups(const vector<CommandGroup>& groups);
  Coordinate& GetBotPosition() { return state.all_bots[0].c; }

  void MoveToCoordinate(int x, int z);
  void MoveToCoordinate(int x, int y, int z);
  void SpawnBotsInGrid(int x0, int x1, int z0, int z1);
  void SpawnBotsInGrid2(int x0, int x1, int z0, int z1);


  vector<CommandGroup> GetMovementGroups(int index);
  void DemolishWholeLayer(int y, int direction);
  void DemolishStrip(int y, int index);

  Solver2D_Demolition(const Matrix& m);
  void Solve(Trace& output);

public:

  static Evaluation::Result Solve(const Matrix& target, Trace& output);
  static void TestSomething();
};
