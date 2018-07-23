#pragma once

#include "../evaluation.h"
#include "../solver.h"
#include "../state.h"
#include "../shortest_dist.h"
#include "../constants.h"
#include "helpers.h"

struct TargetBox {
    vector<Coordinate> corners;
    vector<DistMap> cornerDistances;

    TargetBox(int x, int y, int z, tuple<int, int, int> dim) {
        for (int i = 0; i < 8; ++i) {
            corners.emplace_back(Coordinate{
                x + (i & 1 ? 0 : get<0>(dim)),
                y + (i & 2 ? 0 : get<1>(dim)),
                z + (i & 4 ? 0 : get<2>(dim))});
        }
        sort(corners.begin(), corners.end());
        corners.erase(unique(corners.begin(), corners.end()), corners.end());
    }

    bool cover(int x, int y, int z) {
        return
            corners[0].x <= x && x <= corners.back().x &&
            corners[0].y <= y && y <= corners.back().y &&
            corners[0].z <= z && z <= corners.back().z;
    }

    bool cover(Coordinate c) {
        return cover(c.x, c.y, c.z);
    }
};

class Solver3D_Demolition {
protected:
  Matrix matrix;
  State state;
  vector<TargetBox> targetBoxes;
  int freeBots = TaskConsts::N_BOTS;
  DistMap distToBoundary;

  Solver3D_Demolition(const Matrix& m);
  void Solve();
  void ExecuteCommands(const CommandGroup& group);
  void AllFission();
  void AllFusion();
  void Flip();
  bool DoDemolish();
  bool AllocateBox();
  void InitDistToBoundary();
  bool SomeBoxCovers(int x, int y, int z) {
      for (auto& box : targetBoxes) {
          if (box.cover(x, y, z)) {
              return true;
          }
      }
      return false;
  }

  State::BotState& GetBot(int i) { return state.all_bots[state.active_bots[i]]; };

public:
  static Evaluation::Result Solve(const Matrix& target, Trace& output);
};
