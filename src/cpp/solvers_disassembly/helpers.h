#pragma once

#include "../base.h"
#include "../command.h"

struct XZCoord {
  int x;
  int z;
};

ostream& operator<<(ostream& s, const XZCoord& xz);


struct BotPoint {
  int index;
  int x;
  int z;
};

using BotSquare = vector<BotPoint>;
// using liner notation:
// 0 1
// 2 3

using CommandGroup = vector<Command>;

// Updates the commands into commands each robot needs to do
// they always destory beneath
void SetDemolishSquareCommands(const BotSquare& square, vector<Command>& bot_commands);

vector<int> GetSMovesByOneAxis(int start, int finish);

// useful to make other bots sleep
void AddWaitCommands(vector<Command>& result, int n);

// Creates bots at current spot and moves to another point
// assume current and next are within long liner distance
vector<CommandGroup> SpawnBotAndMove(XZCoord current, XZCoord next, int num_active, int M=0);


// Inverser for very specific use case when we spawn bots from 0 and in linear order
class Inverser {
public:
  Inverser(int _last_bot_id) {
    last_bot_id = _last_bot_id;
  }
  CommandGroup InverseForBot0(const CommandGroup& command_group);
  static CommandGroup InverseForAllBots(const CommandGroup& command_group);

  static void InverseDirection(CoordinateDifference& cd) {
    cd.dx *= -1;
    cd.dy *= -1;
    cd.dz *= -1;
  }
private:
  int last_bot_id;

};
