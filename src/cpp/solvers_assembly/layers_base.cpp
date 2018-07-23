#include "layers_base.h"
#include "grounder.h"
#include "distance_calculator.h"

AssemblySolverLayersBase::AssemblySolverLayersBase(const Matrix& m, bool e, bool l) : SolverBase(m, l), erase(e) {
    if (erase) {
        state.matrix = m;
        state.backMatrix = m;
    }
    helper_mode = false;
    projectionGrounded = Grounder::IsProjectionGrounded(m);
}

void AssemblySolverLayersBase::SetTargetCoordinate(const Coordinate& c)
{
    state.harmonics = true;
    helper_mode = true;
    targetC = c;
    GetBotPosition() = targetC;
}

bool AssemblySolverLayersBase::NeedChange(const Coordinate& c) const {
    if (matrix.Get(c)) {
        if (erase) {
            return state.matrix.Get(c);
        } else {
            return !state.matrix.Get(c);
        }
    }
    return false;
}

void AssemblySolverLayersBase::Change(const CoordinateDifference& cd) {
    if (!erase) {
        Command m(Command::Fill);
        m.cd1 = cd;
        AddCommand(m);
    } else {
        Command m(Command::Void);
        m.cd1 = cd;
        AddCommand(m);
    }
}

void AssemblySolverLayersBase::SolveZ1_GetRZ(int x, int y, int& z0, int& z1)
{
    int r = matrix.GetR();
    z0 = r, z1 = -1;
    for (int z = 0; z < r; ++z)
    {
        if (NeedChange({x, y, z})) {
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
                Change({0, -1, dz});
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
            if (NeedChange({ix, y, z})) {
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
                Change({dx, -1, 0});
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

///////////////// X ///////////////////


void AssemblySolverLayersBase::SolveX1_GetRZ(int& x0, int& x1, int y, int z)
{
    int r = matrix.GetR();
    x0 = r, x1 = -1;
    for (int x = 0; x < r; ++x)
    {
        if (NeedChange({x, y, z})) {
            x0 = min(x0, x);
            x1 = max(x1, x);
        }
    }
}

void AssemblySolverLayersBase::SolveX1_Fill(int z, int y, bool direction)
{
    Coordinate& bc = GetBotPosition();
    for (;;)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            Coordinate c {bc.x + dx, y, z};
            if (matrix.IsInside(c) && matrix.Get(c))
            {
                Change({dx, -1, 0});
            }
        }
        int x0, x1;
        SolveX1_GetRZ(x0, x1, y, z);
        if (x1 < 0) return; // Nothing to do
        int nextx = (x0 == x1) ? x0 : direction ? x0 + 1 : x1 - 1;
        MoveToCoordinate(nextx, y + 1, z);
    }
}

void AssemblySolverLayersBase::SolveX1(int z, int y)
{
    int x0, x1;
    SolveX1_GetRZ(x0, x1, y, z);
    if (x1 < 0) return; // Nothing to do

    Coordinate& bc = GetBotPosition();
    bool xdirection = (bc.x <= (x0 + x1) / 2);
    int xstart = (x0 == x1) ? x0 : xdirection ? x0 + 1 : x1 - 1;
    MoveToCoordinate(xstart, y + 1, z);
    SolveX1_Fill(z, y, xdirection);
}

void AssemblySolverLayersBase::SolveX3_GetRZ(int& x0, int& x1, int y, int z)
{
    int r = matrix.GetR();
    x0 = r, x1 = -1;
    for (int iz = max(z - 1, 0); iz <= min(z + 1, r - 1); ++iz)
    {
        for (int x = 0; x < r; ++x)
        {
            if (NeedChange({x, y, iz})) {
                x0 = min(x0, x);
                x1 = max(x1, x);
            }
        }
    }
}

void AssemblySolverLayersBase::SolveX3_Fill(int z, int y, bool direction)
{
    Coordinate& bc = GetBotPosition();
    for (;;)
    {
        for (int dz = -1; dz <= 1; ++dz)
        {
            Coordinate c {bc.x, y, z + dz};
            if (matrix.IsInside(c) && matrix.Get(c))
            {
                Change({0, -1, dz});
            }
        }
        int x0, x1;
        SolveX3_GetRZ(x0, x1, y, z);
        if (x1 < 0) return; // Nothing to do
        int nextx = direction ? x0 : x1;
        MoveToCoordinate(nextx, y + 1, z);
    }
}

void AssemblySolverLayersBase::SolveX3(int z, int y)
{
    int x0, x1;
    SolveX3_GetRZ(x0, x1, y, z);
    if (x1 < 0) return; // Nothing to do

    Coordinate& bc = GetBotPosition();
    bool xdirection = (bc.x <= (x0 + x1) / 2);
    int xstart = xdirection ? x0 : x1;
    MoveToCoordinate(xstart, y + 1, z);
    SolveX3_Fill(z, y, xdirection);
}


///////////////////////////////////////

int AssemblySolverLayersBase::GetGreedyEstimation(int x, int y, int z) {
    size_t dummy = 0;
    auto fill = GreedyFill({x, y + 1, z}, true, dummy);
    return 3 * fill - MoveEnergy(GetBotPosition().x - x, GetBotPosition().z - z) - ((fill == 0) ? 1000000 : 0);
}

size_t AssemblySolverLayersBase::GreedyFill(const Coordinate& c0, bool dry, size_t& count) {
    size_t result = 0;
    static const vector<PointXZ> DIRS = {{-1, 0}, {1, 0}, {0, 0}, {0, 1}, {0, -1}};
    for (const auto& dxz : DIRS) {
        Coordinate c = {c0.x + dxz.x, c0.y - 1, c0.z + dxz.z};
        CoordinateDifference cd = c - c0;
        if (matrix.IsInside(c) && NeedChange(c)) {
            ++result;
            if (!dry) {
                assert(GetBotPosition() == c0);
                if (!erase) {
                    Command m(Command::Fill);
                    m.cd1 = cd;
                    AddCommand(m);
                } else {
                    Command m(Command::Void);
                    m.cd1 = cd;
                    AddCommand(m);
                }
                assert(!NeedChange(c));
                assert(count != 0);
                --count;
            }
        }
    }
    return result;
}

size_t AssemblySolverLayersBase::SolveGreedy(int y, size_t& count) {
    int bestX = -1;
    int bestZ = -1;
    static constexpr int INF_ESTIMATION = -1000000;
    int bestEstimation = INF_ESTIMATION;
    size_t count1 = 0;

    for (const auto& xz: matrix.YSlices(y)) {
        if (NeedChange({xz.x, y, xz.z})) {
            ++count1;
            int estimation = GetGreedyEstimation(xz.x, y, xz.z);
            if (estimation > bestEstimation) {
                bestX = xz.x;
                bestZ = xz.z;
                bestEstimation = estimation;
            }
        }
    }

    assert(count1 == count);

    assert(bestEstimation != INF_ESTIMATION);

    MoveToCoordinate(bestX, y + 1, bestZ);
    return GreedyFill(GetBotPosition(), false, count);
}

void AssemblySolverLayersBase::SolveLayer(int y) {
    int r = matrix.GetR();
    // Get box
    int16_t x0 = r, x1 = -1, z0 = r, z1 = -1;
    size_t count = 0;
    for (const auto& xz : matrix.YSlices(y)) {
        if (matrix.Get(xz.x, y, xz.z)) {
            x0 = min(x0, xz.x);
            x1 = max(x1, xz.x);
            z0 = min(z0, xz.z);
            z1 = max(z1, xz.z);
            ++count;
        }
    }
    if (x1 < 0) {
        return;  // Nothing to do
    }

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

    for (int z = z0; z <= z1;) {
        if (z < z1) {
            SolveX3(z + 1, y);
            z += 3;
        } else {
            SolveX1(z, y);
            z += 1;
        }
    }

    snapshots.emplace_back(GetSnapshot());

    ApplySnapshot(snapshot);

    for (int z = z1; z >= z0;) {
        if (z > z0) {
            SolveX3(z - 1, y);
            z -= 3;
        } else {
            SolveX1(z, y);
            z -= 1;
        }
    }

    snapshots.emplace_back(GetSnapshot());

    ApplySnapshot(snapshot);
    while (count) {
        assert(SolveGreedy(y, count));
    }
    snapshots.emplace_back(GetSnapshot());

    SelectBestSnapshot(snapshots);
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
    output.Done();
    return Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);
}

Evaluation::Result AssemblySolverLayersBase::SolveHelper(const Matrix& m, Coordinate first_and_last, Trace& output, bool levitation)
{
    AssemblySolverLayersBase solver(m, false, levitation);
    solver.SetTargetCoordinate(first_and_last);
    solver.Solve(output);
    output.Done();
    return Evaluation::Result(solver.state.correct, solver.state.energy);
}
