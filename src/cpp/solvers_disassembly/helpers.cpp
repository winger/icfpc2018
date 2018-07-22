#include "helpers.h"


vector<int> GetSMovesByOneAxis(int start, int finish) {
  vector<int> result;
  if (start == finish) {
    return result;
  }
  int now = start;
  while (now != finish) {
    int delta = max(-15, min(finish - now, 15));
    result.push_back(delta);
    start += delta;
  }
  return result;
}


void AddWaitCommands(vector<Command>& result, int n) {
  for (int i = 0; i < n; ++i) {
    result.push_back(Command(Command::Wait));
  }
}

CommandGroup Inverser::InverseGroup(const CommandGroup& command_group) {
  auto new_group = command_group;
  // only zero bot does anything
  auto& command = new_group[0];
  if (command.type == Command::Wait) {
    // skip
  } else if (command.type == Command::SMove) {
    InverseDirection(command.cd1);
  } else if (command.type == Command::Fission) {
    Command fusionS(Command::FusionS);
    fusionS.cd1 = command.cd1;

    command = Command(Command::FusionP);
    InverseDirection(command.cd1);
    // now the reference to command became invalid
    new_group.push_back(fusionS);
    --last_bot_id;
  } else {
    cerr << "[ERROR] Unsupported command: " << command;
    assert (false);
  }
  return new_group;
}



void SetDemolishSquareCommands(const BotSquare& square, vector<Command>& bot_commands) {
  vector<int> opposite(4);
  opposite[0] = 3;
  opposite[1] = 2;
  opposite[2] = 1;
  opposite[3] = 0;
  for (int from = 0; from < 4; ++from) {
    int to = opposite[from];
    Command gfill(Command::GFill);
    // always go below
    gfill.cd1 = {0, -1, 0};
    gfill.cd2 = {square[to].x - square[from].x, 0, square[to].z - square[from].z};
    bot_commands[ square[from].index ] = gfill;
  }
}

vector<CommandGroup> SpawnBotAndMove(XZCoord current, XZCoord next, int num_waiters) {
  int dx = next.x - current.x;
  int dz = next.z - current.z;
  assert (std::abs(sign(dx)) + std::abs(sign(dz)) == 1);
  vector<CommandGroup> result;
  {
    Command c(Command::SMove);
    c.cd1 = {sign(dx), 0, sign(dz)};
    CommandGroup tmp;
    tmp.push_back(c);
    AddWaitCommands(tmp, num_waiters);
    result.push_back(tmp);
  }
  {
    Command fission(Command::Fission);
    fission.cd1 = {-sign(dx), 0, -sign(dz)};
    fission.m = 1;
    CommandGroup tmp;
    tmp.push_back(fission);
    AddWaitCommands(tmp, num_waiters);
    result.push_back(tmp);
  }
  num_waiters += 1;

  if (dx > 0) {
    Command c(Command::SMove);
    c.cd1 = {0, 0, 0};
    for (int x : GetSMovesByOneAxis(current.x + sign(dx), next.x)) {
        c.cd1.dx = x;

        CommandGroup tmp;
        tmp.push_back(c);
        AddWaitCommands(tmp, num_waiters);
        result.push_back(tmp);
    }
  } else {
    Command c(Command::SMove);
    c.cd1 = {0, 0, 0};
    for (int z : GetSMovesByOneAxis(current.z + sign(dz), next.z)) {
        c.cd1.dz = z;

        CommandGroup tmp;
        tmp.push_back(c);
        AddWaitCommands(tmp, num_waiters);
        result.push_back(tmp);
    }
  }
  return result;
}
