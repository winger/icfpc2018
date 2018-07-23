#include "gravitated.h"

#include "../timer.h"
#include "../constants.h"
#include "../botnets/layer_net.h"

SolverGravitated::SolverGravitated(const Matrix& m)
{
    matrix = m;
}

void SolverGravitated::Order() {
  int size = matrix.GetR();

  order.push_back(std::vector<int>());

  for (int x = 0; x < size; ++x) {
    for (int z = 0; z < size; ++z) {
      if (matrix.Get(x, 0, z)) {
        int index = matrix.Index(x, 0, z);
        order.back().push_back(index);
      }
    }
  }

  const std::vector< std::vector<int> > DIRS_2D {
    {-1, 0, 0},
    {1, 0, 0},
    {0, 0, -1},
    {0, 0, 1}
  };

  std::unordered_set<int> was;

  for (int y = 1; y < size; ++y) {
    order.push_back(std::vector<int>());
    int layerSize = 0;
    for (int x = 0; x < size; ++x) {
      for (int z = 0; z < size; ++z) {
        layerSize += matrix.Get(x, y, z);
        if (matrix.Get(x, y - 1, z) &&
            matrix.Get(x, y, z)) {
          int index = matrix.Index(x, y, z);
          order.back().push_back(index);
          was.insert(index);
        }
      }
    }
    if (order.back().size() == 0) {
      order.pop_back();
      break;

      //cerr << "Level = " << y << endl;
    }
    while (true) {
      auto const& lastLayer = order.back();
      std::vector<int> nextLayer;
      for (int ind: lastLayer) {
        auto cd = matrix.Reindex(ind);
        for (auto const& dir: DIRS_2D) {
          auto nc = cd;
          for (int i = 0; i < nc.size(); ++i) {
            nc[i] += dir[i];
          }
          auto nindex = matrix.Index(nc[0], nc[1], nc[2]);
          if (matrix.IsInside(nc[0], nc[1], nc[2])
             && matrix.Get(nindex)
             && was.find(nindex) == was.end()) {
            nextLayer.push_back(nindex);
            was.insert(nindex);
          }
        }
      }
      if (nextLayer.empty()) {
        if (layerSize != was.size()) {
          cerr << "[ERR] Ungrounded" << endl;
          throw std::runtime_error("Layer Ungrounded model");
        }
        was.clear();
        break;
      }
      order.push_back(nextLayer);
    }
  }
}

void SolverGravitated::Solve(Trace& output)
{
    output.commands.resize(0);
    LayerNet botnet(matrix.GetR());
    // 1. Run BFS to designate order of data feeling
    //    1.1. BFS runs level by level
    // 2. Run bots level by level to draw the picture
    // 3. Finish

    // 1. BFS from the bottom to the top
    Order();

    // 2. Run bots ...
    botnet.Spread(output);
    int y = -1;
    int stepCnt = 0;
    for (auto const& layer: order) {
      auto cd = matrix.Reindex(layer[0]);
      int destY = cd[1] + 1;
      if (y != destY) {
        y = destY;
        botnet.LevelUp(destY, output);
      }
      botnet.CoverPatch(layer, output);
    }
    // 3. Finish
    botnet.Merge(output);
    botnet.Origin(output);
    botnet.Halt(output);
    output.Done();
}

uint64_t SolverGravitated::Solve(const Matrix& m, Trace& output)
{
    SolverGravitated solver(m);
    solver.Solve(output);
    return 0;
}
