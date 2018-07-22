#include "2d_demolition.h"


Solver2D_Demolition::Solver2D_Demolition(const Matrix& m)
{
  matrix = m;
}

void Solver2D_Demolition::Solve(Trace& output) {
  output.commands.resize(0);
  output.commands.push_back(Command(Command::Flip));
  // 1. Move to start of bounding box
  // 2. Spawn bots in a gird
  // 3. Go layer by layer and demolish it
  // 4. Despawn back
  AddCommand(Command(Command::Halt));
}



Evaluation::Result Solver2D_Demolition::Solve(const Matrix& m, Trace& output)
{
    Solver2D_Demolition solver(m);
    solver.Solve(output);
    return Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);
}
