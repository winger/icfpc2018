#include "cube_demolition_tuned.h"
#include "helpers.h"
#include "../constants.h"

namespace {
constexpr bool debug = false;
}


void SolverCubeDemolition_Tuned::DemolishCube() {
  vector<int> opposite(8, -1);
  opposite[0] = 3;
  opposite[1] = 6;
  opposite[2] = 7;
  opposite[3] = 0;
  opposite[4] = 5;
  opposite[5] = 4;
  opposite[6] = 1;
  opposite[7] = 2;

  vector<BotPoint> bottom_square;
  bottom_square.push_back({0, x0, z0});
  bottom_square.push_back({5, x1, z0});
  bottom_square.push_back({6, x1, z1});
  bottom_square.push_back({7, x0, z1});

  vector<BotPoint> top_square;
  top_square.push_back({1, x0, z0});
  top_square.push_back({2, x1, z0});
  top_square.push_back({3, x1, z1});
  top_square.push_back({4, x0, z1});

  vector<Coordinate> cube(8);
  for (const auto& point : bottom_square) {
    cube[ point.index ] = {point.x, 0, point.z};
  }
  for (const auto& point : top_square) {
    cube[ point.index ] = {point.x, y1, point.z};
  }

  CommandGroup bot_commands(8);
  for (int from = 0; from < 8; ++from) {
    int to = opposite[from];
    assert(to >= 0);

    Command gvoid(Command::GVoid);
    int dx = cube[to].x - cube[from].x;
    int dy = cube[to].y - cube[from].y;
    int dz = cube[to].z - cube[from].z;

    gvoid.cd1 = {sign(dx), 0, sign(dz)};
    gvoid.cd2 = {dx, dy, dz};
    bot_commands[ from ] = gvoid;
  }
  ExecuteCommands(bot_commands);
}

void SolverCubeDemolition_Tuned::Solve(Trace& output) {
  output.commands.resize(0);

  // compute bounding box
  int r = matrix.GetR();
  x0 = r;
  x1 = -1;
  y0 = r;
  y1 = -1;
  z0 = r;
  z1 = -1;
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

  if (x1 <= TaskConsts::FAR_COORD_DIFF + 1 && z1 <= TaskConsts::FAR_COORD_DIFF + 1)
  {
    // Small bounding box, no need to move
    x0 = 1;
    z0 = 1;
  }

  if (x1 - x0 == 0 || z1 - z0 == 0 || y1 - y0 == 0) {
    cout << "1 dimension is not supported";
    throw UnsupportedException();
  }

  // for now make work only for 30x30
  if (x1 - x0 > TaskConsts::FAR_COORD_DIFF || z1 - z0 > TaskConsts::FAR_COORD_DIFF || y1 - y0 > TaskConsts::FAR_COORD_DIFF) {
    cout << "dx = " << x1 - x0 << ", " << "dz = " << z1 - z0 << endl;
    throw UnsupportedException();
  }

  if (debug) {
    cout << "Bounding box:" << endl;
    cout << "x = " << x0 << " <-> " << x1 << endl;
    cout << "y = " << y0 << " <-> " << y1 << endl;
    cout << "z = " << z0 << " <-> " << z1 << endl;
  }


  // 1. Move to start of bounding box
  MoveToCoordinateXZ(x0 - 1, z0 - 1);
  // 2. Spawn bots in a gird
  SpawnGrid(x0 - 1, x1 + 1, z0 - 1, z1 + 1);
  PullUpKurwa();

  DemolishCube();
  Shrink(x0 - 1, x1 + 1, z0 - 1, z1 + 1);

  // 6. Move to origin
  MoveToCoordinateXZ(0, 0);

  // AddCommand(Command(Command::Flip));
  AddCommand(Command(Command::Halt));
  output = state.trace;
}


// Happy copy-pasta from AssemblySolverLayersBase
void SolverCubeDemolition_Tuned::MoveToCoordinateXZ(int x, int z)
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

void SolverCubeDemolition_Tuned::ExecuteCommands(const CommandGroup& group) {
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

void SolverCubeDemolition_Tuned::ExecuteCommandGroups(const vector<CommandGroup>& groups) {
  for (const auto& g : groups) {
    ExecuteCommands(g);
  }
}

void SolverCubeDemolition_Tuned::PullUpKurwa() {
  auto moves = GetSMovesByOneAxis(1, y1);
  for (int y_tmp : moves) {
    CommandGroup bot_commands(8, Command(Command::Wait));
    vector<int> ids = {1, 2, 3, 4};
    for (int id : ids) {
      Command c(Command::SMove);
      c.cd1 = {0, y_tmp, 0};
      bot_commands[id] = c;
    }
    ExecuteCommands(bot_commands);
  }
}

void SolverCubeDemolition_Tuned::Shrink(int xmin, int xmax, int zmin, int zmax) {
  auto moves = GetSMovesByOneAxis(y1, 1);
  for (int y_tmp : moves) {
    CommandGroup bot_commands(8, Command(Command::Wait));
    vector<int> ids = {1, 2, 3, 4};
    for (int id : ids) {
      Command c(Command::SMove);
      c.cd1 = {0, y_tmp, 0};
      bot_commands[id] = c;
    }
    ExecuteCommands(bot_commands);
  }

  vector<int> z_base = {0, 1, 2, 5};
  vector<int> z_fork = {7, 4, 3, 6};
  auto z_moves = GetSMovesByOneAxis(zmax, zmin + 1);
  for (int z_tmp : z_moves) {
    CommandGroup bot_commands(state.active_bots.size(), Command(Command::Wait));
    for (int id : z_fork) {
      Command c(Command::SMove);
      c.cd1 = {0, 0, z_tmp};
      bot_commands[BotPosition(state, id)] = c;
    }
    ExecuteCommands(bot_commands);
  }

  CommandGroup fusion_group_z(8);
  for (int i = 0; i < z_base.size(); ++i)
  {
    Command fusionP(Command::FusionP);
    fusionP.cd1 = {0, 0, 1};
    fusion_group_z[ BotPosition(state, z_base[i]) ] = fusionP;
  }
  for (int i = 0; i < z_fork.size(); ++i)
  {
    Command fusionS(Command::FusionS);
    fusionS.cd1 = {0, 0, -1};
    fusion_group_z[ BotPosition(state, z_fork[i]) ] = fusionS;
  }
  ExecuteCommands(fusion_group_z);


  vector<int> x_base = {0, 1};
  vector<int> x_fork = {5, 2};
  auto x_moves = GetSMovesByOneAxis(xmax, xmin + 1);
  for (int x_tmp : x_moves) {
    CommandGroup bot_commands(state.active_bots.size(), Command(Command::Wait));
    for (int id : x_fork) {
      Command c(Command::SMove);
      c.cd1 = {x_tmp, 0, 0};
      bot_commands[BotPosition(state, id)] = c;
    }
    ExecuteCommands(bot_commands);
  }

  CommandGroup fusion_group_x(4);
  for (int i = 0; i < x_base.size(); ++i)
  {
    Command fusionP(Command::FusionP);
    fusionP.cd1 = {1, 0, 0};
    fusion_group_x[ BotPosition(state, x_base[i]) ] = fusionP;
  }
  for (int i = 0; i < x_fork.size(); ++i)
  {
    Command fusionS(Command::FusionS);
    fusionS.cd1 = {-1, 0, 0};
    fusion_group_x[ BotPosition(state, x_fork[i]) ] = fusionS;
  }
  ExecuteCommands(fusion_group_x);

  CommandGroup fusion_group_y;
  {
    Command fusionP(Command::FusionP);
    fusionP.cd1 = {0, 1, 0};
    fusion_group_y.push_back(fusionP);
  }
  {
    Command fusionS(Command::FusionS);
    fusionS.cd1 = {0, -1, 0};
    fusion_group_y.push_back(fusionS);
  }
  ExecuteCommands(fusion_group_y);
}

void SolverCubeDemolition_Tuned::SpawnGrid(int xmin, int xmax, int zmin, int zmax) {
  Command fission_up(Command::Fission);
  fission_up.cd1 = {0, 1, 0};
  fission_up.m = 3;
  CommandGroup group_up;
  group_up.push_back(fission_up);
  ExecuteCommands(group_up);

  Command fission_x(Command::Fission);
  fission_x.cd1 = {1, 0, 0};
  fission_x.m = 1;
  CommandGroup group_x;
  for (int i = 0; i < state.active_bots.size(); ++i)
  {
    group_x.push_back(fission_x);
  }
  ExecuteCommands(group_x);

  auto x_moves = GetSMovesByOneAxis(xmin + 1, xmax);
  for (int x_tmp : x_moves) {
    CommandGroup bot_commands(state.active_bots.size(), Command(Command::Wait));
    vector<int> ids = {2, 5};
    for (int id : ids) {
      Command c(Command::SMove);
      c.cd1 = {x_tmp, 0, 0};
      bot_commands[BotPosition(state, id)] = c;
    }
    ExecuteCommands(bot_commands);
  }

  Command fission_z(Command::Fission);
  fission_z.cd1 = {0, 0, 1};
  fission_z.m = 0;
  CommandGroup group_z;
  for (int i = 0; i < state.active_bots.size(); ++i)
  {
    group_z.push_back(fission_z);
  }
  ExecuteCommands(group_z);

  auto z_moves = GetSMovesByOneAxis(zmin + 1, zmax);
  for (int z_tmp : z_moves) {
    CommandGroup bot_commands(state.active_bots.size(), Command(Command::Wait));
    vector<int> ids = {3, 4, 6, 7};
    for (int id : ids) {
      Command c(Command::SMove);
      c.cd1 = {0, 0, z_tmp};
      bot_commands[BotPosition(state, id)] = c;
    }
    ExecuteCommands(bot_commands);
  }
}

SolverCubeDemolition_Tuned::SolverCubeDemolition_Tuned(const Matrix& m)
{
  state.Init(m.GetR(), Trace());
  matrix = m;
  // cout << matrix;
}


Evaluation::Result SolverCubeDemolition_Tuned::Solve(const Matrix& m, Trace& output)
{
    SolverCubeDemolition_Tuned solver(m);
    solver.Solve(output);
    output.Done();
    return Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);
}
