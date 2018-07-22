#include "2d_demolition.h"

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
}

void Solver2D_Demolition::TestSomething() {
  ShowVector(SplitCoordinatesForGFill(0, 31));
}


Solver2D_Demolition::Solver2D_Demolition(const Matrix& m)
{
  state.Init(m.GetR(), Trace());
  matrix = m;
  cout << matrix;
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
    cout << "moving to coordinate " << x << " " << y << " " << z;
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
  // 1. Move to start of bounding box
  MoveToCoordinate(x0, y1 + 1, z0);
  // 2. Spawn bots in a gird
  SpawnBotsInGrid(x0, x1, z0, z1);
  // 3. Go layer by layer and demolish it
  // 4. Despawn back
  // 5. Move to origin

  AddCommand(Command(Command::Flip));
  AddCommand(Command(Command::Halt));
}


void Solver2D_Demolition::SpawnBotsInGrid(int x0, int x1, int z0, int z1) {
  assert (x0 != x1);
  assert (z0 != z1);

  auto x_coords = SplitCoordinatesForGFill(x0, x1);
  // this is the width one
  auto z_coords = SplitCoordinatesForGFill(z0, z1);


  assert (x_coords.size() % 2 == 0);
  assert (z_coords.size() % 2 == 0);

  // split into halfs
  {
    Command fission(Command::Fission);
    fission.cd1 = {x0 + 1, 0, 0};
    fission.m = 19;
    AddCommand(fission);
  }
  // the child bot became by id=1
  auto& child_coord = state.all_bots[1].c;

  // transfer chlid bot into his destination
  Command c(Command::SMove);
  c.cd1 = {0, 0, 0};
  for (; child_coord.x != x_coords[1]; )
  {
      // parent move
      state.trace.commands.push_back(Command(Command::Wait));
      c.cd1.dx = max(-15, min(x_coords[1] - child_coord.x, 15));
      state.trace.commands.push_back(c);
      state.Step();
  }

}


Evaluation::Result Solver2D_Demolition::Solve(const Matrix& m, Trace& output)
{
    Solver2D_Demolition solver(m);
    solver.Solve(output);
    return Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);
}
