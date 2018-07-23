#include "non_gravitated.h"

#include "../timer.h"
#include "../constants.h"
#include "../botnets/layer_net.h"

SolverNonGravitated::SolverNonGravitated(const Matrix& m)
{
    matrix = m;
}

void SolverNonGravitated::Order() {
  int size = matrix.GetR();

  for (int y = 0; y < size; ++y) {
    order.push_back(std::vector<int>());
    for (int x = 0; x < size; ++x) {
      for (int z = 0; z < size; ++z) {
        if (matrix.Get(x, y, z)) {
          int index = matrix.Index(x, y, z);
          order.back().push_back(index);
        }
      }
    }
    if (order.back().size() == 0) {
      order.pop_back();
      break;
    }
  }
}

void SolverNonGravitated::Solve(Trace& output, bool smart, bool naive)
{
    output.commands.resize(0);
    LayerNet botnet(matrix.GetR(), smart);

    // 1. Cut by layers
    Order();

    // 2. Run bots ...
    botnet.Spread(output);
    int y = -1;
    int stepCnt = 0;
    for (auto const& layer: order) {
      auto cd = matrix.Reindex(layer[0]);
      int destY = cd[1] + 1;
      if (y != destY) {
        botnet.LevelUp(destY, output);
        if (y == -1) {
          if (!naive) {
            botnet.Flip(output);
          }
        }
        y = destY;
      }
      botnet.CoverPatch(layer, output);
    }
    // 3. Finish
    if (!naive) {
      botnet.Flip(output);
    }
    botnet.Merge(output);
    botnet.Origin(output);
    botnet.Halt(output);
    output.Done();
}

uint64_t SolverNonGravitated::Solve(const Matrix& m, Trace& output, bool smart, bool naive)
{
    SolverNonGravitated solver(m);
    solver.Solve(output, smart, naive);
    return 0;
}
