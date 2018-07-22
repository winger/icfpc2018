#include "2d_demolition.h"
#include "helpers.h"

namespace {
// using step as 1 less that maximum of long move
constexpr int STEP = 29;
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
  for (const auto& c : group) {
    // cout << "adding command " << c << endl;
    state.trace.commands.push_back(c);
  }
  // cout << "state.Step()" << endl;
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
  AddCommand(Command(Command::Flip));

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
  if (x1 - x0 >= 30 || z1 - z0 >= 30) {
    throw StopException();
  }

  assert (x0 + 30 >= x1);
  assert (z0 + 30 >= z1);

  // 1. Move to start of bounding box
  MoveToCoordinate(x0, y1 + 1, z0);
  // 2. Spawn bots in a gird
  // SpawnBotsInGrid(x0, x1, z0, z1);
  // cout << "SpawnBotsInGrid2" << endl;
  SpawnBotsInGrid2(x0, x1, z0, z1);

  // 3. Go layer by layer and demolish it
  // TODO: enable for big use cases
  // PrepareMovementCommands();
  // cout << "Demolish layers" << endl;

  int direction = 1;
  for (int y_layer = y1; y_layer >= 0; --y_layer) {
    DemolishLayer(y_layer, direction);
    direction *= -1;

    // move everyone down by 1 level
    if (y_layer > 0) {
      Command c(Command::SMove);
      c.cd1 = {0, -1, 0};
      for (int i = 0; i < 2 * x_coords.size(); ++i) {
        state.trace.commands.push_back(c);
      }
      state.Step();
    }
  }
  // 4. Despawn back
  ExecuteCommandGroups(despawn_groups);

  // 5. Move to origin
  MoveToCoordinate(0, 0, 0);

  AddCommand(Command(Command::Flip));
  AddCommand(Command(Command::Halt));
  output = state.trace;
}

void Solver2D_Demolition::PrepareMovementCommands() {
  // TODO: this is empty in small use case
}


void Solver2D_Demolition::DoDemolishCommands() {
  // we assume we have only 1 square and 4 bots
  BotSquare square;
  square.push_back({1, x_coords[0], z_coords[0]});
  square.push_back({2, x_coords[0], z_coords[1]});
  square.push_back({0, x_coords[1], z_coords[0]});
  square.push_back({3, x_coords[1], z_coords[1]});

  CommandGroup bot_commands(4);
  SetDemolishSquareCommands(square, bot_commands);

  ExecuteCommands(bot_commands);
}

void Solver2D_Demolition::DemolishLayer(int y, int direction) {
  // TODO: y & direction is unused in small use case
  // so we don't need to move
  DoDemolishCommands();
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
    despawn_groups.push_back(inverser.InverseGroup(all_groups[i]));
  }
}


void Solver2D_Demolition::SpawnBotsInGrid(int x0, int x1, int z0, int z1) {
  // Don't need this anymore
  // assert (x0 != x1);
  // assert (z0 != z1);
  //
  // x_coords = SplitCoordinatesForGFill(x0, x1);
  // // this is the width one
  // z_coords = SplitCoordinatesForGFill(z0, z1);
  //
  // assert (x_coords.size() % 2 == 0);
  // assert (z_coords.size() % 2 == 0);
  // // split into halfs
  // {
  //   Command fission(Command::Fission);
  //   fission.cd1 = {1, 0, 0};
  //   fission.m = 19;
  //   AddCommand(fission);
  // }
  // // the child bot became by id=1
  // auto& child_coord = state.all_bots[1].c;
  //
  // // transfer chlid bot into his destination
  // auto next_moves = GetSMovesByOneAxis(child_coord.x, x_coords[1]);
  // Command c(Command::SMove);
  // c.cd1 = {0, 0, 0};
  // for (auto move : next_moves) {
  //   cd.cd1.dx = move;
  //
  //   state.trace.commands.push_back(Command(Command::Wait));
  //   state.trace.commands.push_back(c);
  //   state.Step();
  // }
  // assert(chlid_coord.x == x_coords[1]);
  //
  // total_other_bots = 0;
  // for (int i = 1; i < z_coords.size(); ++i) {
  //   bool last_one = (i == z_coords.size() - 1);
  //   int next_z = last_one ? z_coords[i] : z_coords[i] + 1;
  //
  //   auto& main_bot = state.all_bots[0];
  //   auto next_z_moves = GetSMovesByOneAxis(main_bot.z, next_z);
  //
  //   Command cz(Command::SMove);
  //   cz.cd1 = {0, 0, 0};
  //   for (auto move : next_z_moves) {
  //     cz.cd1.dz = move;
  //     state.trace.commands.push_back(cz);
  //     state.trace.commands.push_back(cz);
  //     for (int j = 0; j < total_other_bots; ++j) {
  //       state.trace.commands.push_back(Command(Command::Wait));
  //     }
  //     state.Step();
  //   }
  //
  //   // drop the bots
  //   if (!last_one) {
  //     Command fission(Command::Fission);
  //     fission.cd1 = {0, 0, -1};
  //     fission.m = 1;
  //     AddCommand(fission);
  //     AddCommand(fission);
  //     for (int j = 0; j < total_other_bots; ++j) {
  //       state.trace.commands.push_back(Command(Command::Wait));
  //     }
  //     state.Step();
  //     total_other_bots += 2;
  //   }
  // }

  // TODO: verify that everyone is at its position
}


Evaluation::Result Solver2D_Demolition::Solve(const Matrix& m, Trace& output)
{
    Solver2D_Demolition solver(m);
    solver.Solve(output);
    return Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);
}
