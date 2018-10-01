#include "cube_demolition.h"
#include "helpers.h"

namespace {
constexpr bool debug = false;
}


void SolverCubeDemolition::DemolishCube() {
  vector<int> opposite(8, -1);
  opposite[0] = 4;
  opposite[1] = 6;
  opposite[2] = 5;
  opposite[3] = 7;

  opposite[4] = 0;
  opposite[6] = 1;
  opposite[5] = 2;
  opposite[7] = 3;

  vector<BotPoint> bottom_square;
  bottom_square.push_back({1, x0, z0});
  bottom_square.push_back({3, x1, z0});
  bottom_square.push_back({5, x1, z1});
  bottom_square.push_back({0, x0, z1});

  vector<BotPoint> top_square;
  top_square.push_back({2, x0, z0});
  top_square.push_back({4, x1, z0});
  top_square.push_back({6, x1, z1});
  top_square.push_back({7, x0, z1});

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

void SolverCubeDemolition::Solve(Trace& output) {
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
  if (x1 - x0 == 0 || z1 - z0 == 0 || y1 - y0 == 0) {
    cout << "1 dimension is not supported" << endl;
    throw UnsupportedException();
  }

  // for now make work only for 30x30
  if (x1 - x0 > 30 || z1 - z0 > 30 || y1 - y0 > 30) {
    cout << "dx = " << x1 - x0 << ", " << "dz = " << z1 - z0 << endl;
    throw UnsupportedException();
  }
  //
  // assert (x0 + 30 >= x1);
  // assert (z0 + 30 >= z1);
  if (debug) {
    cout << "Bounding box:" << endl;
    cout << "x = " << x0 << " <-> " << x1 << endl;
    cout << "y = " << y0 << " <-> " << y1 << endl;
    cout << "z = " << z0 << " <-> " << z1 << endl;
  }



  // 1. Move to start of bounding box
  MoveToCoordinateXZ(x0 - 1, z0 - 1);
  // 2. Spawn bots in a gird
  SpawnDoubleBotsOnFloor(x0 - 1, x1 + 1, z0 - 1, z1 + 1);
  ReplicateToTop();

  int total_bots_in_layer = 8;
  DemolishCube();
  ShrinkToBottom();
  ExecuteCommandGroups(despawn_groups);

  // 6. Move to origin
  MoveToCoordinateXZ(0, 0);

  // AddCommand(Command(Command::Flip));
  AddCommand(Command(Command::Halt));
  output = state.trace;
}


// Happy copy-pasta from AssemblySolverLayersBase
void SolverCubeDemolition::MoveToCoordinateXZ(int x, int z)
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

void SolverCubeDemolition::ExecuteCommands(const CommandGroup& group) {
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

void SolverCubeDemolition::ExecuteCommandGroups(const vector<CommandGroup>& groups) {
  for (const auto& g : groups) {
    ExecuteCommands(g);
  }
}


void SolverCubeDemolition::ReplicateToTop() {
  CommandGroup replication_group;
  for (int i = 0; i < 4; ++i) {
    Command fission(Command::Fission);
    fission.cd1 = {0, 1, 0};
    fission.m = 0;
    replication_group.push_back(fission);
  }
  ExecuteCommands(replication_group);

  auto moves = GetSMovesByOneAxis(1, y1);
  for (int y_tmp : moves) {
    CommandGroup bot_commands(8, Command(Command::Wait));
    vector<int> ids = {2, 4, 6, 7};
    for (int id : ids) {
      Command c(Command::SMove);
      c.cd1 = {0, y_tmp, 0};
      bot_commands[id] = c;
    }
    ExecuteCommands(bot_commands);
  }
}

void SolverCubeDemolition::ShrinkToBottom() {
  vector<int> top_ids = {2, 4, 6, 7};
  vector<int> bottom_ids = {1, 3, 5, 0};

  auto moves = GetSMovesByOneAxis(y1, 1);
  for (int y_tmp : moves) {
    CommandGroup bot_commands(8, Command(Command::Wait));
    for (int id : top_ids) {
      Command c(Command::SMove);
      c.cd1 = {0, y_tmp, 0};
      bot_commands[id] = c;
    }
    ExecuteCommands(bot_commands);
  }

  CommandGroup fusion_group(8);
  for (int i = 0; i < 4; ++i) {
    {
      Command fusionP(Command::FusionP);
      fusionP.cd1 = {0, 1, 0};
      fusion_group[ bottom_ids[i] ] = fusionP;
    }
    {
      Command fusionS(Command::FusionS);
      fusionS.cd1 = {0, -1, 0};
      fusion_group[ top_ids[i] ] = fusionS;
    }
  }
  ExecuteCommands(fusion_group);
}

void SolverCubeDemolition::SpawnDoubleBotsOnFloor(int xmin, int xmax, int zmin, int zmax) {
  vector <XZCoord> xz_coords;
  xz_coords.push_back({xmin, zmin});
  xz_coords.push_back({xmax, zmin});
  xz_coords.push_back({xmax, zmax});
  xz_coords.push_back({xmin, zmax});

  // for reversing
  vector<CommandGroup> all_groups;

  auto now = xz_coords[0];
  int num_waiters = 0;
  for (int i = 1; i < xz_coords.size(); ++i) {
    // spawn double bots!
    int M = 1;
    auto groups = SpawnBotAndMove(now, xz_coords[i], num_waiters, M);
    now = xz_coords[i];
    num_waiters += 1;
    ExecuteCommandGroups(groups);

    for (const auto& g : groups) {
      all_groups.push_back(g);
    }
  }

  Inverser inverser(3);
  for (int i = all_groups.size() - 1; i >= 0; --i) {
    despawn_groups.push_back(inverser.InverseForBot0(all_groups[i]));
  }
}

SolverCubeDemolition::SolverCubeDemolition(const Matrix& m)
{
  state.Init(m.GetR(), Trace());
  matrix = m;
  // cout << matrix;
}


Evaluation::Result SolverCubeDemolition::Solve(const Matrix& m, Trace& output)
{
    SolverCubeDemolition solver(m);
    solver.Solve(output);
    output.Done();
    return Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);
}
