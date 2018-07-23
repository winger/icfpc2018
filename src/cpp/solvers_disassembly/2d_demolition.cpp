#include "2d_demolition.h"
#include "helpers.h"

namespace {
constexpr bool debug = false;
}

namespace {
// using step as 1 less that maximum of long move
constexpr int STEP = 28;
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

void ShowVector(vector<int> v) {
  cout << "v = { ";
  for (const auto& value : v) {
    cout << value << " ";
  }
  cout << "}" << endl;
}

} // namespace



void Solver2D_Demolition::TestSomething() {
  ShowVector(SplitCoordinatesForGFill(0, 31));
}

void Solver2D_Demolition::ExecuteCommands(const CommandGroup& group) {
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

void Solver2D_Demolition::ExecuteCommandGroups(const vector<CommandGroup>& groups) {
  for (const auto& g : groups) {
    ExecuteCommands(g);
  }
}

Solver2D_Demolition::Solver2D_Demolition(const Matrix& m)
{
  state.Init(m.GetR(), Trace());
  matrix = m;
  // cout << matrix;
}

// Happy copy-pasta from AssemblySolverLayersBase
void Solver2D_Demolition::MoveToCoordinate(int x, int z)
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

void Solver2D_Demolition::MoveToCoordinate(int x, int y, int z)
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

void Solver2D_Demolition::Solve(Trace& output) {
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
  // for now make work only for 30x30
  // if (x1 - x0 >= 30 || z1 - z0 >= 30) {
  //   cout << "dx = " << x1 - x0 << ", " << "dz = " << z1 - z0 << endl;
  //   throw StopException();
  // }
  //
  // assert (x0 + 30 >= x1);
  // assert (z0 + 30 >= z1);

  // 1. Move to start of bounding box
  MoveToCoordinate(x0, y1 + 1, z0);
  // 2. Spawn bots in a gird
  // SpawnBotsInGrid(x0, x1, z0, z1);
  // cout << "SpawnBotsInGrid2" << endl;
  SpawnBotsInGrid2(x0, x1, z0, z1);

  // after we spawned bots we can set ids for them
  for (int i = 0; i < total_bots_in_layer / 2; ++i) {
    first_row.push_back(i + 1);
    second_row.push_back(total_bots_in_layer - i);
  }
  assert (second_row[0] == total_bots_in_layer);
  second_row[0] = 0;

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
  // 4. Despawn back
  // cout << "Despawning bots" << endl;

  // "nice" hack to return into despawnable state
  if (y1 % 2 == 0) {
    if (x_coords.size() >= 4) {
      auto groups = GetMovementGroups(x_coords.size() - 4);
      for (int i = int(groups.size()) - 1; i >= 0; --i) {
        auto reversed_group = Inverser::InverseForAllBots(groups[i]);
        ExecuteCommands(reversed_group);
      }
    }
  }

  ExecuteCommandGroups(despawn_groups);
  // cout << "Done Despawning bots" << endl;

  // 5. Move to origin
  MoveToCoordinate(0, 0, 0);

  // AddCommand(Command(Command::Flip));
  AddCommand(Command(Command::Halt));
  output = state.trace;
}

void showVector(const vector<int>& vv) {
  cout << "{ ";
  for (auto v : vv) {
    cout << v << " ";
  }
  cout << "}" << endl;
}


// index is where smallest x stays
vector<CommandGroup> Solver2D_Demolition::GetMovementGroups(int index) {
  assert (index + 3 < x_coords.size());
  auto first_moves = GetSMovesByOneAxis(x_coords[index], x_coords[index + 2]);
  auto second_moves = GetSMovesByOneAxis(x_coords[index + 1], x_coords[index + 3]);
  // showVector(first_moves);
  // showVector(second_moves);
  // showVector(x_coords);
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

void Solver2D_Demolition::DemolishStrip(int y, int index) {
  CommandGroup bot_commands(total_bots_in_layer);
  for (int z_index = 0; z_index < z_coords.size(); z_index += 2) {
    BotSquare square;
    square.push_back({first_row[z_index],      x_coords[index],     z_coords[z_index]});
    square.push_back({first_row[z_index + 1],  x_coords[index],     z_coords[z_index + 1]});
    square.push_back({second_row[z_index],     x_coords[index + 1], z_coords[z_index]});
    square.push_back({second_row[z_index + 1], x_coords[index + 1], z_coords[z_index + 1]});
    SetDemolishSquareCommands(square, bot_commands);
  }

  ExecuteCommands(bot_commands);
}



void Solver2D_Demolition::DemolishWholeLayer(int y, int direction) {
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


void Solver2D_Demolition::SpawnBotsInGrid2(int x0, int x1, int z0, int z1) {
  assert (x0 != x1);
  assert (z0 != z1);

  x_coords = SplitCoordinatesForGFill(x0, x1);
  // this is the width one
  z_coords = SplitCoordinatesForGFill(z0, z1);

  assert (x_coords.size() % 2 == 0);
  assert (z_coords.size() % 2 == 0);

  vector <XZCoord> xz_coords;
  for (auto z : z_coords) {
    xz_coords.push_back({x_coords[0], z});
  }
  for (int i = z_coords.size() - 1; i >= 0; --i) {
    int z = z_coords[i];
    xz_coords.push_back({x_coords[1], z});
  }
  total_bots_in_layer = xz_coords.size();
  assert (total_bots_in_layer % 4 == 0);

  vector<CommandGroup> all_groups;

  auto now = xz_coords[0];
  int num_waiters = 0;
  for (int i = 1; i < xz_coords.size(); ++i) {
    auto groups = SpawnBotAndMove(now, xz_coords[i], num_waiters);
    now = xz_coords[i];
    num_waiters += 1;
    ExecuteCommandGroups(groups);

    for (const auto& g : groups) {
      all_groups.push_back(g);
    }


    // TODO: verify current position of robot and number of active ones
  }

  // Now robots should be like:
  // 1  2  3  4  ...
  // 0 10  9  8  ...

  // also set the reversed commands
  Inverser inverser(3);
  for (int i = all_groups.size() - 1; i >= 0; --i) {
    despawn_groups.push_back(inverser.InverseForBot0(all_groups[i]));
  }
}


void Solver2D_Demolition::SpawnBotsInGrid(int x0, int x1, int z0, int z1) {
}


Evaluation::Result Solver2D_Demolition::Solve(const Matrix& m, Trace& output)
{
    Solver2D_Demolition solver(m);
    solver.Solve(output);
    output.Done();
    return Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);
}
