#include "layer_net.h"

#include "../command.h"
#include "../constants.h"

LayerNet::LayerNet(int size, bool smart) {
    matrix.Init(size);
    this->smart = smart;
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
  ids[0] = bots[1].id;
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
        //bot.Print();
        auto pBotCmd = bot.Fission(1, 0, 0, amnt);
        cmds[bot.id] = pBotCmd.second;

        auto& botp = pBotCmd.first;
        //cerr << botp.id << " - " << (amnt + 1) / 2 << " " << nc << endl;
        pids[botp.x] = std::make_pair(nc, botp.id);
        bots[botp.id] = botp;
      }
    }
    CleanCmds(output, cmds);
    bool go = false;
    for (auto const& kv: ids) {
      cmds[kv.second] = Command(Command::Wait);
    }
    for (auto const& kv: pids) {
      int dx = kv.second.first;
      int delta = dx - kv.first;
      int bid = kv.second.second;
      Command cmd(Command::Wait);
      if (delta != 0) {
        go |= true;
        cmd = Command(Command::SMove);
        cmd.cd1 = {delta, 0, 0};
      }
      cmds[bid] = cmd;
      ids[dx] = bid;
      bots[bid].x = dx;
      coords.push_back(dx);
    }
    std::sort(coords.begin(), coords.end());
    if (go) {
      CleanCmds(output, cmds);
    }
  }
}

std::pair<LayerBot, Command> LayerBot::Fission(int dx, int dy, int dz, int m) {
  if (!Nd(dx, dy, dz)) {
    throw std::runtime_error("LB/Fission/Not nd");
  }
  LayerBot bot;
  bot.x = x + dx;
  bot.y = y + dy;
  bot.z = z + dz;

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


// Invariant z == 0 || z == R - 1
void LayerNet::Relocate(Trace& output, std::vector<int> const& chunk) {
  // 1. Select who needs to move
  // 2. Assign destinations
  // 3. Lift them up
  // 4. Move them where they need to be
  // 4. Put them backwards
  int size = matrix.GetR();
  // 1. SELECT
  std::set<int> dests;
  //cerr << "Chunk: ";
  for (int x: chunk) {
    dests.insert(x);
  }

  std::set<int> stable;
  std::map<int /* x */, int /* id */> movableBots;
  for (auto const& kv: bots) {
    movableBots[bots[kv.first].x] = kv.second.id;
  }
  std::set<int> toLeave;
  std::set<int> toMove = dests;
  int zstatus = 0;
  for (auto const& kv: bots) {
    auto const& bot = kv.second;
    zstatus = bot.z;
    if (dests.find(bot.x) != dests.end()) {
      stable.insert(bot.id);
      toLeave.insert(bot.x);
      toMove.erase(bot.x);
      movableBots.erase(bot.x);
    }
  }

  //cerr << "toMove: "; for (auto v: toMove) { cerr << v << " "; } cerr << endl;
  // 2. ASSIGN
  std::map<int /* id */, int /* dx */> destsX;
  std::map<int /* id */, int /* dz */> destsZ;
  auto it = movableBots.begin();
  {
    int step = 0;
    for (auto dx: toMove) {
      ++step;
      destsX[it->second] = dx;
      destsZ[it->second] = bots[it->second].z == 0 ? step : size - 1 - step;
      if (destsZ[it->second] < 0 || destsZ[it->second] >= size) {
        throw std::runtime_error("LN/Relocate/Out of place");
      }
      ++it;
    }
  }
  while (it != movableBots.end()) {
    stable.insert(it->second);
    ++it;
  }
  //cerr << "3: "; for (auto const& kv: bots) { cerr << "(" << kv.second.x << " / " << kv.second.z << ") "; } cerr << endl;
  // 3. LIFT THEM UP
  while (true) {
    bool stillGo = false;
    std::map<int /* bid */, Command> cmds;
    for (auto const& id: stable) {
      cmds[id] = Command(Command::Wait);
    }
    for (auto const& kv: destsZ) {
      int bid = kv.first;
      int destZ = kv.second;
      auto& bot = bots[bid];
      //cerr << bot.x << " " << bot.z << endl;
      auto cmd = bot.MoveTowards(bot.x, bot.y, destZ);

      cmds[bid] = cmd;
      stillGo |= cmd.type != Command::Wait;
    }
    if (not stillGo) { break; }
    CleanCmds(output, cmds);
  }
  //cerr << "4: "; for (auto const& kv: bots) { cerr << "(" << kv.second.x << " / " << kv.second.z << ") "; } cerr << endl;
  // 4. MOVE THEM TO DEST POS
  /*
  cerr << "4. ";
  for (auto const& kv: bots) {
    cerr << kv.second.x << " ";
  }
  cerr << endl;*/

  while (true) {
    bool stillGo = false;
    std::map<int /* bid */, Command> cmds;
    for (auto const& id: stable) {
      cmds[id] = Command(Command::Wait);
    }
    for (auto const& kv: destsX) {
      int bid = kv.first;
      int destX = kv.second;
      auto& bot = bots[bid];
      auto cmd = bot.MoveTowards(destX, bot.y, bot.z);
      cmds[bid] = cmd;
      stillGo |= cmd.type != Command::Wait;
    }
    if (not stillGo) { break; }
    CleanCmds(output, cmds);
  }
  //cerr << "5: "; for (auto const& kv: bots) { cerr << "(" << kv.second.x << " / " << kv.second.z << ") "; } cerr << endl;
  // 5. PUT THEM DOWN

  while (true) {
    bool stillGo = false;
    std::map<int /* bid */, Command> cmds;
    for (auto id: stable) {
      cmds[id] = Command(Command::Wait);
    }
    for (auto const& kv: destsZ) {
      int bid = kv.first;
      auto& bot = bots[bid];
      auto cmd = bot.MoveTowards(bot.x, bot.y, zstatus);
      cmds[bid] = cmd;
      stillGo |= cmd.type != Command::Wait;
    }
    if (not stillGo) { break; }
    CleanCmds(output, cmds);
  }
  //cerr << "F: "; for (auto const& kv: bots) { cerr << "(" << kv.second.x << " / " << kv.second.z << ") "; } cerr << endl;
  //assert(false);
}

void LayerNet::Cover(
    Trace& output,
    std::map<int /* x */, std::vector<int>>& xzPlane) {

  std::map<int /* x */, int /* id */> pos;
  int zstatus = 0;
  for (auto const& kv: bots) {
    pos[kv.second.x] = kv.first;
    zstatus = kv.second.z;
  }
  int destZ = zstatus == 0 ? matrix.GetR() - 1 : 0;
  //cerr << destZ << endl;
  std::vector<BotCoverTask> tasks;
  for (auto const& kv: pos) {
    if (xzPlane.find(kv.first) == xzPlane.end()) {
      auto& tmp = xzPlane[kv.first];
    }
    tasks.push_back(BotCoverTask(xzPlane[kv.first], kv.second, destZ));
    xzPlane[kv.first].clear();;
  }
  while (true) {

    bool stillGo = false;
    std::map<int /* bid */, Command> cmds;
    for (auto& task: tasks) {
      if (smart) {
        stillGo |= task.DoSmartStep(cmds, bots);
      } else {
        stillGo |= task.DoStupidStep(cmds, bots);
      }
    }

    if (stillGo) {
      CleanCmds(output, cmds);
    } else {
      break;
    }
  }
}

bool BotCoverTask::DoStupidStep(
    std::map<int /* bid */, Command>& cmds,
    std::map<int /* id */, LayerBot>& bots) {
  auto& bot = bots[bid];
  Command cmd;
  if (points.size() == 0) {
    cmd = bot.MoveTowards(bot.x, bot.y, destZ);
  } else {
    if (points[0] == bot.z) {
      cmd = Command(Command::Fill);
      cmd.cd1 = {0, -1, 0};
      points.erase(points.begin());
    } else {
      cmd = bot.MoveTowards(bot.x, bot.y, points[0]);
    }
  }
  cmds[bid] = cmd;
  return cmd.type != Command::Wait;
}

bool BotCoverTask::DoSmartStep(
    std::map<int /* bid */, Command>& cmds,
    std::map<int /* id */, LayerBot>& bots) {

  return false;
}

void LayerNet::CoverPatch(std::vector<int> layer, Trace& output) {
  std::set<int> xs;
  std::vector<int> xvs;
  std::map<int /* x */, std::vector<int>> xzPlane;
  for (auto v: layer) {
    auto coord = matrix.Reindex(v);
    xs.insert(coord[0]);
    auto& vv = xzPlane[coord[0]];
    vv.push_back(coord[2]);
  }
  for (auto v: xs) {
    xvs.push_back(v);
  }

  int chunkSize = bots.size();
  std::sort(xvs.begin(), xvs.end());
  for (int i = 0; i < xvs.size(); i += chunkSize) {
    int curChunkSize = min(chunkSize, (int)xvs.size() - i);
    std::vector<int> chunk(curChunkSize);
    std::copy(
      xvs.begin() + i,
      xvs.begin() + i + curChunkSize,
      chunk.begin());
    //cerr << "Chunk: "; for (auto v: chunk) { cerr << v << " "; } cerr << endl;
    Relocate(output, chunk);
    Cover(output, xzPlane);
  }
}

void LayerNet::Merge(Trace& output) {
  int size = matrix.GetR();
  std::vector<std::vector<int>> cnt(size);
  for (auto const& kv: bots) {
    auto const& bot = kv.second;
    cnt[bot.x].push_back(bot.id);
  }
  std::vector<int> indicies;
  for (int i = 0; i < size; ++i) {
    if (cnt[i].size()) {
      indicies.push_back(i);
      std::sort(
        cnt[i].begin(),
        cnt[i].end(),
        [this](int lhs, int rhs) {
          return bots[lhs].z < bots[rhs].z;
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
  // cerr << "Second Part" << endl;
  // Second part
  std::vector<int /* id */> sline;
  for (auto ind: indicies) {
    sline.push_back(cnt[ind][0]);
  }
  std::sort(
    sline.begin(),
    sline.end(),
    [this](int lhs, int rhs) {
      return bots[lhs].x < bots[rhs].x;
    }
  );

  while (bots.size() > 1) {
    std::map<int /* bid */, Command> cmds;
    ShrinkLine(cmds, sline, 0);
    CleanCmds(output, cmds);
  }
}

void LayerNet::Origin(Trace& output) {
  if (bots.size() != 1) {
    throw std::runtime_error("LN/Origin fiasca, bratan");
  }
  auto& bot = bots.begin()->second;
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

inline int filterLong(int x) {
  x = max(x, -TaskConsts::LONG_LIN_DIFF);
  return min(x, TaskConsts::LONG_LIN_DIFF);
}

Command LayerBot::MoveTowards(int destX, int destY, int destZ) {
  int dx = filterLong(destX - x);
  int dy = filterLong(destY - y);
  int dz = filterLong(destZ - z);
  if (dx != 0) { return SMoveUC(dx, 0, 0); }
  if (dy != 0) { return SMoveUC(0, dy, 0); }
  if (dz != 0) { return SMoveUC(0, 0, dz); }
  return Command(Command::Wait);
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
  cfst.cd1 = {-dx, -dy, -dz};
  cmds[prm.id] = cfst;

  Command csnd(Command::FusionS);
  csnd.cd1 = {dx, dy, dz};
  cmds[snd.id] = csnd;

  assert(abs(dx) + abs(dy) + abs(dz) > 0);

  for (auto v: snd.seeds) {
    prm.seeds.push_back(v);
  }
  prm.seeds.push_back(snd.id);
  std::sort(prm.seeds.begin(), prm.seeds.end());
}

int LayerNet::ShrinkLine(
    std::map<int /* bid */, Command>& cmds,
    std::vector<int /* int */>& line,
    int axe) {
  int totalWait = 0;
  if (line.size() == 1) {
    auto& bot = bots[line[0]];
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
      cmds[strap[0]] = Command(Command::Wait);
    }
    for (; start < strap.size(); start += 2) {
      auto& fst = bots[strap[start]];
      auto& snd = bots[strap[start + 1]];
      if (fst.Coord(axe) + 1 == snd.Coord(axe)) {
        //fst.Print();
        //snd.Print();
        LayerBot::Fusion(cmds, fst, snd);
        strap.erase(strap.begin() + start + 1);
        bots.erase(snd.id);
        --start;
      } else {
        //cerr << snd.Coord(axe) << " -- " << fst.Coord(axe) << endl;
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

void LayerBot::Print() const {
  cout << "Id " << id << endl;
  cout << "Coords " << x << " " << y << " " << z << endl;
  cout << "Seeds: ";
  for (auto v: seeds) {
    cout << v << " ";
  }
  cout << endl;
}
