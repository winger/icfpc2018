#include "layers_base.h"
#include "grounder.h"

AssemblySolverLayersBase::AssemblySolverLayersBase(const Matrix& m, bool e, bool l) : matrix(m), erase(e), levitation(l) {
    state.Init(m.GetR(), Trace());
    if (erase) {
        state.matrix = m;
        state.backMatrix = m;
    }
    helper_mode = false;
    projectionGrounded = Grounder::IsProjectionGrounded(m);
    target = {0, 0, 0};
}

void AssemblySolverLayersBase::SetTargetCoordinate(const Coordinate& c)
{
    state.harmonics = true;
    helper_mode = true;
    target = c;
    GetBotPosition() = target;
}

void AssemblySolverLayersBase::MoveToCoordinate(int x, int z)
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

void AssemblySolverLayersBase::MoveToCoordinate(int x, int y, int z, bool finalize)
{
    Coordinate& bc = GetBotPosition();
    Command c(Command::SMove);
    c.cd1 = {0, 0, 0};
    if (finalize)
    {
        MoveToCoordinate(x, z);
        for (; bc.y != y; )
        {
            c.cd1.dy = max(-15, min(y - bc.y, 15));
            AddCommand(c);
        }
    }
    else
    {
        for (; bc.y != y; )
        {
            c.cd1.dy = max(-15, min(y - bc.y, 15));
            AddCommand(c);
        }
        MoveToCoordinate(x, z);
    }
}

void AssemblySolverLayersBase::SolveInit() {
    if (!helper_mode) {
        if (!projectionGrounded) {
            if (levitation) {
                AddCommand(Command(Command::Flip));
            }
        }
    }
}

void AssemblySolverLayersBase::SolveZ1_GetRZ(int x, int y, int& z0, int& z1)
{
    int r = matrix.GetR();
    z0 = r, z1 = -1;
    for (int z = 0; z < r; ++z)
    {
        if ((!erase && matrix.Get(x, y, z) && !state.matrix.Get(x, y, z)) || (erase && matrix.Get(x, y, z) && state.matrix.Get(x, y, z))) {
            z0 = min(z0, z);
            z1 = max(z1, z);
        }
    }
}

void AssemblySolverLayersBase::SolveZ1_Fill(int x, int y, bool direction)
{
    Coordinate& bc = GetBotPosition();
    for (;;)
    {
        for (int dz = -1; dz <= 1; ++dz)
        {
            Coordinate c {x, y, bc.z + dz};
            if (matrix.IsInside(c) && matrix.Get(c))
            {
                if (!erase) {
                    Command m(Command::Fill);
                    m.cd1 = {0, -1, dz};
                    AddCommand(m);
                } else {
                    Command m(Command::Void);
                    m.cd1 = {0, -1, dz};
                    AddCommand(m);
                }
            }
        }
        int z0, z1;
        SolveZ1_GetRZ(x, y, z0, z1);
        if (z1 < 0) return; // Nothing to do
        int nextz = (z0 == z1) ? z0 : direction ? z0 + 1 : z1 - 1;
        MoveToCoordinate(x, y + 1, nextz);
    }
}

void AssemblySolverLayersBase::SolveZ1(int x, int y)
{
    int z0, z1;
    SolveZ1_GetRZ(x, y, z0, z1);
    if (z1 < 0) return; // Nothing to do

    Coordinate& bc = GetBotPosition();
    bool zdirection = (bc.z <= (z0 + z1) / 2);
    int zstart = (z0 == z1) ? z0 : zdirection ? z0 + 1 : z1 - 1;
    MoveToCoordinate(x, y + 1, zstart);
    SolveZ1_Fill(x, y, zdirection);
}

void AssemblySolverLayersBase::SolveZ3_GetRZ(int x, int y, int& z0, int& z1)
{
    int r = matrix.GetR();
    z0 = r, z1 = -1;
    for (int ix = max(x - 1, 0); ix <= min(x + 1, r - 1); ++ix)
    {
        for (int z = 0; z < r; ++z)
        {
            if ((!erase && matrix.Get(ix, y, z) && !state.matrix.Get(ix, y, z)) || (erase && matrix.Get(ix, y, z) && state.matrix.Get(ix, y, z))) {
                z0 = min(z0, z);
                z1 = max(z1, z);
            }
        }
    }
}

void AssemblySolverLayersBase::SolveZ3_Fill(int x, int y, bool direction)
{
    Coordinate& bc = GetBotPosition();
    for (;;)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            Coordinate c {x + dx, y, bc.z};
            if (matrix.IsInside(c) && matrix.Get(c))
            {
                if (!erase) {
                    Command m(Command::Fill);
                    m.cd1 = {dx, -1, 0};
                    AddCommand(m);
                } else {
                    Command m(Command::Void);
                    m.cd1 = {dx, -1, 0};
                    AddCommand(m);
                }
            }
        }
        int z0, z1;
        SolveZ3_GetRZ(x, y, z0, z1);
        if (z1 < 0) return; // Nothing to do
        int nextz = direction ? z0 : z1;
        MoveToCoordinate(x, y + 1, nextz);
    }
}

void AssemblySolverLayersBase::SolveZ3(int x, int y)
{
    int z0, z1;
    SolveZ3_GetRZ(x, y, z0, z1);
    if (z1 < 0) return; // Nothing to do

    Coordinate& bc = GetBotPosition();
    bool zdirection = (bc.z <= (z0 + z1) / 2);
    int zstart = zdirection ? z0 : z1;
    MoveToCoordinate(x, y + 1, zstart);
    SolveZ3_Fill(x, y, zdirection);
}

StateSnapshot AssemblySolverLayersBase::GetSnapshot() {
    return {matrix, state};
}

void AssemblySolverLayersBase::ApplySnapshot(const StateSnapshot& s) {
    matrix = s.matrix;
    state = s.state;
}

void AssemblySolverLayersBase::SelectBestSnapshot(const StateSnapshots& s) {
    size_t best_energy = s[0].state.energy;
    size_t best_index = 0;
    for (size_t i = 1; i < s.size(); ++i) {
        assert(s[i].matrix == s[0].matrix);
        if (s[i].state.energy < best_energy) {
            best_energy = s[i].state.energy;
            best_index = i;
        }
    }
    ApplySnapshot(s[best_index]);
}

void AssemblySolverLayersBase::SolveLayer(int y)
{
    int r = matrix.GetR();
    // Get box
    int x0 = r, x1 = -1, z0 = r, z1 = -1;
    for (int x = 0; x < r; ++x)
    {
        for (int z = 0; z < r; ++z)
        {
            if (matrix.Get(x, y, z))
            {
                x0 = min(x0, x);
                x1 = max(x1, x);
                z0 = min(z0, z);
                z1 = max(z1, z);
            }
        }
    }
    if (x1 < 0) return; // Nothing to do

    Coordinate c = state.all_bots[0].c;

    StateSnapshots snapshots;

    auto snapshot = GetSnapshot();

    for (int x = x0; x <= x1;) {
        if (x < x1) {
            SolveZ3(x + 1, y);
            x += 3;
        } else {
            SolveZ1(x, y);
            x += 1;
        }
    }

    snapshots.emplace_back(GetSnapshot());

    ApplySnapshot(snapshot);

    for (int x = x1; x >= x0;) {
        if (x > x0) {
            SolveZ3(x - 1, y);
            x -= 3;
        } else {
            SolveZ1(x, y);
            x -= 1;
        }
    }

    snapshots.emplace_back(GetSnapshot());
    SelectBestSnapshot(snapshots);
}

void AssemblySolverLayersBase::SolveFinalize() {
    if (helper_mode) {
        MoveToCoordinate(target.x, target.y, target.z, true);
    } else {
        if (!projectionGrounded) {
            if (levitation) {
                AddCommand(Command(Command::Flip));
            }
        }
        MoveToCoordinate(target.x, target.y, target.z, true);
        AddCommand(Command(Command::Halt));
    }
}

void AssemblySolverLayersBase::Solve(Trace& output) {
    SolveInit();
    if (!erase) {
        for (int i = 0; i + 1 < matrix.GetR(); ++i) {
            SolveLayer(i);
        }
    } else {
        for (int i = matrix.GetR() - 2; i >= 0; --i) {
            SolveLayer(i);
        }
    }
    SolveFinalize();
    output = state.trace;
}

Evaluation::Result AssemblySolverLayersBase::Solve(const Matrix& m, Trace& output, bool erase, bool levitation)
{
    AssemblySolverLayersBase solver(m, erase, levitation);
    solver.Solve(output);
    return Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);
}

Evaluation::Result AssemblySolverLayersBase::SolveHelper(const Matrix& m, Coordinate first_and_last, Trace& output, bool levitation)
{
    AssemblySolverLayersBase solver(m, false, levitation);
    solver.SetTargetCoordinate(first_and_last);
    solver.Solve(output);
    return Evaluation::Result(solver.state.correct, solver.state.energy);
}
