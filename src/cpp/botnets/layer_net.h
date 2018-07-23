#pragma once

#include "../solver.h"
#include "../state.h"

struct LayerBot {
  int x, y, z;
  int id;
  std::vector<int> seeds;

  int Coord(int axe);

  void CheckLine(Matrix const& m, int dx, int dy, int dz);
  void SMoveUC(Trace& output, int dx, int dy, int dz);
  Command SMoveUC(int dx, int dy, int dz);
  Command MoveTowards(int destX, int destY, int destZ);
  std::pair<LayerBot, Command> Fission(int dx, int dy, int dz, int m);

  static void Fusion(
    std::map<int /* bid */, Command>& cmds,
    LayerBot& prm,
    LayerBot& snd);

  void Print() const;

  static bool Nd(int x, int y, int z);

  static std::vector<int> ByAxeDist(int axe, int dist);
};

struct BotCoverTask {
  std::vector<int> points;
  std::vector<std::pair<int, int>> segments;
  int bid;
  int bidp{-1};
  int destZ;
  int state{0};

  BotCoverTask(std::vector<int> pointsz, int bidz, int destZz)
    : points(pointsz), bid(bidz), destZ(destZz) {
    precompute();
  }

  void precompute();

  bool DoStupidStep(
    std::map<int /* bid */, Command>& cmds,
    std::map<int /* id */, LayerBot>& bots);

  bool DoSmartStep(
    std::map<int /* bid */, Command>& cmds,
    std::map<int /* id */, LayerBot>& bots);
};

class LayerNet
{
protected:
  Matrix matrix;
  std::map<int /* id */, LayerBot> bots;
  bool smart{false};

  static void CleanCmds(
    Trace& output,
    std::map<int /* bid */, Command>& cmds);

  int ShrinkLine(
    std::map<int /* bid */, Command>& cmds,
    std::vector<int /* id */>& line,
    int axe);
public:
  LayerNet(int size, bool smart);

  // Invariant z == 0 || z == R - 1
  void Relocate(Trace& output, std::vector<int> const& chunk);

  void Cover(
      Trace& output,
      std::map<int /* x */, std::vector<int>>& xzPlane);

  void Spread(Trace& output); // DONE
  void LevelUp(int level, Trace& output);  // DONE
  void CoverPatch(std::vector<int> layer, Trace& output);
  void Merge(Trace& output); // // DONE
  void Origin(Trace& output); // // DONE
  void Halt(Trace& output); // // DONE
  void Flip(Trace& output);
};
