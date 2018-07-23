#include "2d_demolition_tuned.h"
#include "helpers.h"
#include "../constants.h"
#include "../state.h"

namespace {
constexpr bool debug = false;
}

namespace {
// using step as 1 less that maximum of long move
constexpr int STEP = TaskConsts::FAR_COORD_DIFF - 2;
// splits the segment sequence of pair points,
// so that there is at most STEP distance between them and at least 1
// the last one could on distance of STEP + 1
vector <int> SplitCoordinatesForGFill(int a, int b) {
  vector<int> result;
  while (true) {
    assert (a != b);
    // the equality is okay here as this is the last one
    if (b <= a + STEP + 1) {
      result.push_back(a);
      result.push_back(b);
      break;
    }
    result.push_back(a);
    result.push_back(a + STEP);
    a += STEP + 1;
  }
  return result;
}

} // namespace



void Solver2D_Demolition_Tuned::TestSomething() {
  showVector(SplitCoordinatesForGFill(0, 31));
}

void Solver2D_Demolition_Tuned::ExecuteCommands(const CommandGroup& group) {
  int index = 0;
  for (const auto& c : group) {
    if (debug) {
      cout << index << ": adding command " << c << endl;
    }
    state.trace.commands.push_back(c);
    index += 1;
  }
  if (debug) {
    cout << "state.Step()" << endl;
  }
  state.Step();
}

void Solver2D_Demolition_Tuned::ExecuteCommandGroups(const vector<CommandGroup>& groups) {
  for (const auto& g : groups) {
    ExecuteCommands(g);
  }
}

Solver2D_Demolition_Tuned::Solver2D_Demolition_Tuned(const Matrix& m)
{
  state.Init(m.GetR(), Trace());
  matrix = m;
  // cout << matrix;
}

// Happy copy-pasta from AssemblySolverLayersBase
void Solver2D_Demolition_Tuned::MoveToCoordinate(int x, int z)
{
    Coordinate& bc = GetBotPosition();
    Command c(Command::SMove);
    for (; abs(x - bc.x) > 5; )
    {
        c.cd1 = {max(-15, min(x - bc.x, 15)), 0, 0};
        AddCommand(c);
    }
    for (; abs(z - bc.z) > 5; )
    {
        c.cd1 = {0, 0, max(-15, min(z - bc.z, 15))};
        AddCommand(c);
    }
    if (bc.x == x)
    {
        if (bc.z == z)
        {
            // Already here
        }
        else
        {
            c.cd1 = {0, 0, z - bc.z};
            AddCommand(c);
        }
    }
    else
    {
        if (bc.z == z)
        {
            c.cd1 = {x - bc.x, 0, 0};
            AddCommand(c);
        }
        else
        {
            c.type = Command::LMove;
            c.cd1 = {x - bc.x, 0, 0};
            c.cd2 = {0, 0, z - bc.z};
            AddCommand(c);
        }
    }
}

void Solver2D_Demolition_Tuned::MoveToCoordinate(int x, int y, int z)
{
    // cout << "moving to coordinate " << x << " " << y << " " << z << endl;
    Coordinate& bc = GetBotPosition();
    Command c(Command::SMove);
    c.cd1 = {0, 0, 0};
    for (; bc.y != y; )
    {
        c.cd1.dy = max(-15, min(y - bc.y, 15));
        AddCommand(c);
    }
    MoveToCoordinate(x, z);
}

void Solver2D_Demolition_Tuned::Solve(Trace& output) {
  output.commands.resize(0);

  // compute bounding box
  int r = matrix.GetR();
  int x0 = r;
  int x1 = -1;
  int y0 = r;
  int y1 = -1;
  int z0 = r;
  int z1 = -1;
  for (int x = 0; x < r; ++x) {
    for (int y = 0; y < r; ++y) {
        for (int z = 0; z < r; ++z) {
          if (matrix.Get(x, y, z))
          {
              x0 = min(x0, x);
              x1 = max(x1, x);
              y0 = min(y0, y);
              y1 = max(y1, y);
              z0 = min(z0, z);
              z1 = max(z1, z);
          }
      }
    }
  }
  
  // 1. Move to start of bounding box
  MoveToCoordinate(x0, y1 + 1, z0);
  // 2. Spawn bots in a gird
  // SpawnBotsInGrid(x0, x1, z0, z1);
  // cout << "SpawnBotsInGrid2" << endl;
  SpawnBotsInGrid(x0, x1, z0, z1);

  // 3. Go layer by layer and demolish it
  // cout << "Demolish layers" << endl;
  int direction = 1;
  bool flip_done = false;

  {
    CommandGroup bots_flip;
    bots_flip.push_back(Command(Command::Flip));
    for (int i = 1; i < total_bots_in_layer; ++i) {
      bots_flip.push_back(Command(Command::Wait));
    }
    ExecuteCommands(bots_flip);
  }

  for (int y_layer = y1; y_layer >= 0; --y_layer) {

    if (y_layer == 0) {
      // do a group flip
      CommandGroup bots_flip;
      bots_flip.push_back(Command(Command::Flip));
      for (int i = 1; i < total_bots_in_layer; ++i) {
        bots_flip.push_back(Command(Command::Wait));
      }
      ExecuteCommands(bots_flip);
      flip_done = true;
    }

    DemolishWholeLayer(y_layer, direction);
    direction *= -1;

    // move everyone down by 1 level
    if (y_layer > 0) {
      CommandGroup bots_down;
      Command c(Command::SMove);
      c.cd1 = {0, -1, 0};
      for (int i = 0; i < total_bots_in_layer; ++i) {
        // cout << "Adding DOWN command" << endl;
        bots_down.push_back(c);
      }
      ExecuteCommands(bots_down);
    }
  }
  assert (flip_done);
  
  // 4. Despawn
  Despawn(y1);
  
  // 5. Move to origin
  MoveToCoordinate(0, 0, 0);

  // AddCommand(Command(Command::Flip));
  AddCommand(Command(Command::Halt));
  output = state.trace;
}

// index is where smallest x stays
vector<CommandGroup> Solver2D_Demolition_Tuned::GetMovementGroups(int index) {
  assert (index + 3 < x_coords.size());
  auto first_moves = GetSMovesByOneAxis(x_coords[index], x_coords[index + 2]);
  auto second_moves = GetSMovesByOneAxis(x_coords[index + 1], x_coords[index + 3]);
  // showVector(first_moves);
  // showVector(second_moves);
  // showVector(x_coords);
  // showVector(z_coords);
  // cout << "index = " << index << endl;
  assert (first_moves.size() >= second_moves.size());


  vector<CommandGroup> result;
  for (int i = 0; i < first_moves.size(); ++i) {
    CommandGroup bot_commands(total_bots_in_layer);
    for (const auto id : first_row) {
      Command c(Command::SMove);
      c.cd1 = {first_moves[i], 0, 0};
      bot_commands[id] = c;
    }

    for (const auto id : second_row) {
      if (i >= second_moves.size()) {
        bot_commands[id] = Command(Command::Wait);
      } else {
        Command c(Command::SMove);
        c.cd1 = {second_moves[i], 0, 0};
        bot_commands[id] = c;
      }
    }
    result.push_back(bot_commands);
  }
  return result;
}

void Solver2D_Demolition_Tuned::DemolishStrip(int y, int index) {
  CommandGroup bot_commands;
  for (int i = 0; i < total_bots_in_layer; ++i)
  {
    bot_commands.push_back(Command(Command::Wait)); 
  }
  bool demolish = false;
  for (int z_index = 0; z_index < z_coords.size(); z_index += 2) {
    bool empty = true;
    for (int x = x_coords[index]; x <= x_coords[index + 1] && empty; ++x)
    {
      for (int z = z_coords[z_index]; z <= z_coords[z_index + 1] && empty; ++z)
      {
        if (matrix.Get(x, y, z))
        {
          empty = false;
        }
      }
    }
    if (!empty)
    {
      demolish = true;
      BotSquare square;
      square.push_back({first_row[z_index],      x_coords[index],     z_coords[z_index]});
      square.push_back({first_row[z_index + 1],  x_coords[index],     z_coords[z_index + 1]});
      square.push_back({second_row[z_index],     x_coords[index + 1], z_coords[z_index]});
      square.push_back({second_row[z_index + 1], x_coords[index + 1], z_coords[z_index + 1]});
      SetDemolishSquareCommands(square, bot_commands);
    }
  }

  if (demolish)
  {
    ExecuteCommands(bot_commands);
  }
}

void Solver2D_Demolition_Tuned::DemolishWholeLayer(int y, int direction) {
  if (direction == 1) {
    // cout << "Going forward" << endl;
    DemolishStrip(y, 0);
    for (int index = 0; index + 2 < x_coords.size(); index += 2) {
      auto groups = GetMovementGroups(index);
      ExecuteCommandGroups(groups);
      DemolishStrip(y, index + 2);
    }
  } else {
    // cout << "Going backward" << endl;
    DemolishStrip(y, x_coords.size() - 2);
    for (int index = x_coords.size() - 4; index >= 0; index -= 2) {
      auto groups = GetMovementGroups(index);
      for (int i = int(groups.size()) - 1; i >= 0; --i) {
        auto reversed_group = Inverser::InverseForAllBots(groups[i]);
        ExecuteCommands(reversed_group);
      }
      DemolishStrip(y, index);
    }
  }
}

void Solver2D_Demolition_Tuned::SpawnBotsInGrid(int x0, int x1, int z0, int z1) 
{
  assert (x0 != x1);
  assert (z0 != z1);

  x_coords = SplitCoordinatesForGFill(x0, x1);
  // this is the width one
  z_coords = SplitCoordinatesForGFill(z0, z1);

  assert (x_coords.size() % 2 == 0);
  assert (z_coords.size() % 2 == 0);

  total_bots_in_layer = z_coords.size() * 2;

  vector<CommandGroup> all_groups;

  first_row.push_back(0);

  int num_waiters = 0;
  for (int i = 1; i < z_coords.size(); ++i) 
  {
    vector<CommandGroup> groups;
    CommandGroup fork_group;
    for (int j = 0; j < num_waiters; ++j)
    {
      fork_group.push_back(Command(Command::Wait));
    }
    Command fork(Command::Fission);
    fork.cd1 = {0, 0, 1};
    fork.m = (z_coords.size() - i) * 2 - 1;
    fork_group.push_back(fork);
    groups.push_back(fork_group);
    num_waiters++;

    first_row.push_back(state.all_bots[first_row.back()].seeds[0]);

    if (i & 1)
    {
      // need to move odds
      auto moves = GetSMovesByOneAxis(z_coords[i - 1] + 1, z_coords[i]);
      for (int move: moves)
      {
        CommandGroup move_group;
        for (int j = 0; j < num_waiters; ++j)
        {
          move_group.push_back(Command(Command::Wait));
        }
        Command c(Command::SMove);
        c.cd1 = {0, 0, move};
        move_group.push_back(c);
        groups.push_back(move_group);
      }
    }

    ExecuteCommandGroups(groups);

    for (const auto& g : groups) 
    {
      all_groups.push_back(g);
    }
  }

  vector<CommandGroup> front_groups;
  CommandGroup fork_group(first_row.size());
  for (const auto& i: first_row) 
  {
    int pos = BotPosition(state, i);
    Command fork(Command::Fission);
    fork.cd1 = {1, 0, 0};
    fork.m = 0;
    fork_group[pos] = fork;
    second_row.push_back(state.all_bots[i].seeds[0]);
  }
  front_groups.push_back(fork_group);

  auto moves = GetSMovesByOneAxis(x_coords[0] + 1, x_coords[1]);
  for (int move: moves)
  {
    CommandGroup move_group(total_bots_in_layer);
    for (const auto& i: first_row) 
    {
      move_group[i] = Command(Command::Wait);
    }

    for (const auto& i: second_row) 
    {
      Command c(Command::SMove);
      c.cd1 = {move, 0, 0};
      move_group[i] = c;
    }
    front_groups.push_back(move_group);
  }

  ExecuteCommandGroups(front_groups);
  // Now robots should be like:
  // 9  8  7  6  ...
  // 0  1  2  3  ...
}

void Solver2D_Demolition_Tuned::Despawn(int y)
{
  auto moves = (y & 1) 
      ? GetSMovesByOneAxis(x_coords[1], x_coords[0] + 1)
      : GetSMovesByOneAxis(x_coords[x_coords.size() - 1], x_coords[x_coords.size() - 2] + 1);
  vector<CommandGroup> front_groups;
  for (int move: moves)
  {
    CommandGroup move_group(total_bots_in_layer);
    for (const auto& i: first_row) 
    {
      move_group[i] = Command(Command::Wait);
    }

    for (const auto& i: second_row) 
    {
      Command c(Command::SMove);
      c.cd1 = {move, 0, 0};
      move_group[i] = c;
    }
    front_groups.push_back(move_group);
  }
  ExecuteCommandGroups(front_groups);

  CommandGroup fusions_x(total_bots_in_layer);
  for (int i = 0; i < first_row.size(); ++i) 
  {
    Command fusionP(Command::FusionP);
    fusionP.cd1 = {1, 0, 0};
    fusions_x[ BotPosition(state, first_row[i]) ] = fusionP;
  }
  for (int i = 0; i < second_row.size(); ++i) 
  {
    Command fusionS(Command::FusionS);
    fusionS.cd1 = {-1, 0, 0};
    fusions_x[ BotPosition(state, second_row[i]) ] = fusionS;
  }
  ExecuteCommands(fusions_x);

  int num_waiters = total_bots_in_layer / 2 - 1;
  for (int i = z_coords.size() - 1; i > 0; --i)
  {
    if (i & 1)
    {
      auto moves = GetSMovesByOneAxis(z_coords[i], z_coords[i - 1] + 1);
      vector<CommandGroup> groups;
      for (int move: moves)
      {
        CommandGroup move_group;
        for (int j = 0; j < num_waiters; ++j) 
        {
          move_group.push_back(Command(Command::Wait));
        }

        Command c(Command::SMove);
        c.cd1 = {0, 0, move};
        move_group.push_back(c);
        groups.push_back(move_group);
      }
      ExecuteCommandGroups(groups);
    }
    
    num_waiters--;
    CommandGroup fusion_group;
    for (int j = 0; j < num_waiters; ++j) 
    {
      fusion_group.push_back(Command(Command::Wait));
    }
    Command fusionP(Command::FusionP);
    fusionP.cd1 = {0, 0, 1};
    fusion_group.push_back(fusionP);
    Command fusionS(Command::FusionS);
    fusionS.cd1 = {0, 0, -1};
    fusion_group.push_back(fusionS);

    ExecuteCommands(fusion_group);
  }
}

Evaluation::Result Solver2D_Demolition_Tuned::Solve(const Matrix& m, Trace& output)
{
    Solver2D_Demolition_Tuned solver(m);
    solver.Solve(output);
    output.Done();
    return Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);
}
