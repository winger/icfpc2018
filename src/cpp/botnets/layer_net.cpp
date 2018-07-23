#include "layer_net.h"

#include "../command.h"
#include "../constants.h"

LayerNet::LayerNet(int size) {
    matrix.Init(size);
    LayerBot bot;
    bot.id = 1;
    bot.x = bot.y = bot.z = 0;
    for (int i = 2; i <= TaskConsts::N_BOTS; ++i) {
      bot.seeds.push_back(i);
    }
    bots[bot.id] = bot;
}

void LayerNet::Spread(Trace& output) {
  int full = min(2 * matrix.GetR(), TaskConsts::N_BOTS);
  int half = full / 2;

  std::vector<int /* x */> coords{0, half};
  std::map<int /* x */, int /* id */> ids;
  std::map<int /* x */, std::pair<int /* dx */, int /* id */>> pids;
  while (ids.size() != half) {
    std::map<int /* bid */, Command> cmds;

    for (int i = 0; i < coords.size() - 1; ++i) {
      int delta = coords[i + 1] - coords[i];
      if (delta == 1) {
        cmds[ids[coords[i]]] = Command(Command::Wait);
      } else {
        auto nc = coords[i] + delta / 2;
        int amnt = (coords[i + 1] - nc) * 2 - 1;

        auto& bot = bots[ids[coords[i]]];
        auto pBotCmd = bot.Fission(1, 0, 0, amnt);

        auto& botp = pBotCmd.first;
        pids[botp.x] = std::make_pair(nc, botp.id);
        bots[botp.id] = botp;
        cmds[bot.id] = pBotCmd.second;
      }
    }
    CleanCmds(output, cmds);
    for (auto const& kv: ids) {
      cmds[kv.second] = Command(Command::Wait);
    }
    for (auto const& kv: pids) {
      int delta = kv.second.first - kv.first;
      auto cmd = Command(Command::SMove);
      cmd.cd1 = {delta, 0, 0};
      cmds[kv.second.second] = cmd;
    }
    CleanCmds(output, cmds);
  }
}

std::pair<LayerBot, Command> LayerBot::Fission(int dx, int dy, int dz, int m) {
  if (!Nd(dx, dy, dz)) {
    throw std::runtime_error("LB/Fission/Not nd");
  }
  LayerBot bot;
  bot.x = x + dx;
  bot.y = x + dy;
  bot.z = x + dz;

  bot.id = seeds[0];
  for (int i = 0; i < m; ++i) {
    bot.seeds.push_back(seeds[i + 1]);
  }
  seeds.erase(seeds.begin(), seeds.begin() + m + 1);

  auto cmd = Command(Command::Fission);
  cmd.cd1 = {1, 0, 0};
  cmd.m = m;

  return std::make_pair(bot, cmd);
}

void LayerNet::LevelUp(int level, Trace& output) {
  for (auto& kv: bots) {
    auto& bot = kv.second;
    int dy = level - bot.y;
    bot.CheckLine(matrix, 0, dy, 0);
    bot.SMoveUC(output, 0, dy, 0);
  }
}

void LayerNet::CoverPatch(std::vector<int> layer, Trace& output) {
  // min (dx, dz)
  // preset bot by straps
  // move forward bots, covering under them
  //
}

void LayerNet::Merge(Trace& output) {
  int size = matrix.GetR();
  std::vector<std::vector<LayerBot>> cnt(size);
  for (auto const& kv: bots) {
    auto bot = kv.second;
    cnt[bot.x].push_back(bot);
  }
  std::vector<int> indicies;
  for (int i = 0; i < size; ++i) {
    if (cnt[i].size()) {
      indicies.push_back(i);
      std::sort(
        cnt[i].begin(),
        cnt[i].end(),
        [](LayerBot const& lhs, LayerBot const& rhs) {
          return lhs.z < rhs.z;
        });
    }
  }

  while (true) {
    std::map<int /* bid */, Command> cmds;
    int totalNonWait = bots.size();
    for (auto ind: indicies) {
      totalNonWait -= ShrinkLine(cmds, cnt[ind], 2);
    }
    if (totalNonWait == 0) {
      break;
    } else {
      CleanCmds(output, cmds);
    }
  }

  // Second part
  std::vector<LayerBot> sline;
  for (auto ind: indicies) {
    sline.push_back(cnt[ind][0]);
  }
  std::sort(
    sline.begin(),
    sline.end(),
    [](LayerBot const& lhs, LayerBot const& rhs) {
      return lhs.x < rhs.x;
    }
  );
  while (bots.size() > 1) {
    std::map<int /* bid */, Command> cmds;
    ShrinkLine(cmds, sline, 2);
    CleanCmds(output, cmds);
  }
}

void LayerNet::Origin(Trace& output) {
  if (bots.size() != 1) {
    throw std::runtime_error("LN/Origin fiasca, bratan");
  }
  auto& bot = bots[0];
  while (bot.x != 0) {
    auto dist = min(bot.x, TaskConsts::LONG_LIN_DIFF);
    bot.CheckLine(matrix, -dist, 0, 0);
    bot.SMoveUC(output, -dist, 0, 0);
  }
  while (bot.z != 0) {
    auto dist = min(bot.z, TaskConsts::LONG_LIN_DIFF);
    bot.CheckLine(matrix, 0, 0, -dist);
    bot.SMoveUC(output, 0, 0, -dist);
  }
  while (bot.y != 0) {
    auto dist = min(bot.y, TaskConsts::LONG_LIN_DIFF);
    bot.CheckLine(matrix, 0, -dist, 0);
    bot.SMoveUC(output, 0, -dist, 0);
  }
}

void LayerNet::Halt(Trace& output) {
  output.commands.push_back(Command(Command::Halt));
}

void LayerNet::CleanCmds(
    Trace& output,
    std::map<int /* bid */, Command>& cmds) {
  for (auto const& kv: cmds) {
    output.commands.push_back(kv.second);
  }
  cmds.clear();
}

void LayerBot::SMoveUC(Trace& output, int dx, int dy, int dz) {
  output.commands.push_back(SMoveUC(dx, dy, dz));
}

Command LayerBot::SMoveUC(int dx, int dy, int dz) {
  x += dx;
  y += dy;
  z += dz;

  Command cmd(Command::SMove);
  cmd.cd1 = {dx, dy, dz};
  return cmd;
}

int sgn(int a) {
  if (a) {
    return a > 0 ? 1 : -1;
  }
  return 0;
}

void LayerBot::CheckLine(Matrix const& m, int dx, int dy, int dz) {
  int dim = (dx ? 1 : 0) + (dy ? 1 : 0) + (dz ? 1 : 0);
  if (dim > 1) {
    throw std::runtime_error("LB/CheckLine; Not a line");
  }
  int lin = dx + dy + dz;
  int sx = sgn(dx);
  int sy = sgn(dy);
  int sz = sgn(dz);
  for (int i = 0; i < abs(lin); ++i) {
    if (m.Get(x + sx * i, y + sy * i, z + sz * i)) {
      throw std::runtime_error("LB/CheckLine; Obstacle on a line");
    }
  }
}

void LayerBot::Fusion(
  std::map<int /* bid */, Command>& cmds,
  LayerBot& prm,
  LayerBot& snd) {

  int dx = prm.x - snd.x;
  int dy = prm.y - snd.y;
  int dz = prm.z - snd.z;
  if (!Nd(dx, dy, dz)) {
    throw std::runtime_error("LB/Fusion/To far");
  }
  Command cfst(Command::FusionP);
  cfst.cd1 = {dx, dy, dz};
  cmds[prm.id] = cfst;

  Command csnd(Command::FusionS);
  csnd.cd1 = {-dx, -dy, -dz};
  cmds[snd.id] = csnd;

  for (auto v: snd.seeds) {
    prm.seeds.push_back(v);
  }
  prm.seeds.push_back(snd.id);
  std::sort(prm.seeds.begin(), prm.seeds.end());
}

int LayerNet::ShrinkLine(
    std::map<int /* bid */, Command>& cmds,
    std::vector<LayerBot> line,
    int axe) {
  int totalWait = 0;
  if (line.size() == 1) {
    auto& bot = line[0];
    if (bot.Coord(axe) == 0) {
      cmds[bot.id] = Command(Command::Wait);
      totalWait++;
    } else {
      auto dist = min(bot.Coord(axe), TaskConsts::LONG_LIN_DIFF);
      auto ldf = LayerBot::ByAxeDist(axe, -dist);
      bot.CheckLine(matrix, ldf[0], ldf[1], ldf[2]);
      cmds[bot.id] = bot.SMoveUC(ldf[0], ldf[1], ldf[2]);
    }
  } else {
    auto& strap = line;
    int start = strap.size() % 2;
    if (start == 1) {
      cmds[strap[0].id] = Command(Command::Wait);
    }
    for (; start < strap.size(); start += 2) {
      auto& fst = strap[start];
      auto& snd = strap[start + 1];
      if (fst.Coord(axe) + 1 == snd.Coord(axe)) {
        LayerBot::Fusion(cmds, fst, snd);
        strap.erase(strap.begin() + start + 1);
        bots.erase(snd.id);
        --start;
      } else {
        auto dist = min(snd.Coord(axe) - fst.Coord(axe) - 1, TaskConsts::LONG_LIN_DIFF);
        auto ldf = LayerBot::ByAxeDist(axe, -dist);
        snd.CheckLine(matrix, ldf[0], ldf[1], ldf[2]);
        cmds[snd.id] = snd.SMoveUC(ldf[0], ldf[1], ldf[2]);
        cmds[fst.id] = Command(Command::Wait);
      }
    }
  }
  return totalWait;
}

int LayerBot::Coord(int axe) {
  if (axe == 0) { return x; }
  if (axe == 1) { return y; }
  if (axe == 2) { return z; }
  throw std::runtime_error("LB/Coord/Axe");
}

std::vector<int> LayerBot::ByAxeDist(int axe, int dist) {
  std::vector<int> result(3);
  for (int i = 0; i < 3; ++i) {
    result[i] = axe == i ? dist : 0;
  }
  return result;
}

bool LayerBot::Nd(int dx, int dy, int dz) {
  return abs(dx) + abs(dy) + abs(dz) <= 2 &&
    abs(dx) <= 1 &&
    abs(dy) <= 1 &&
    abs(dz) <= 1;
}
