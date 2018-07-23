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
    target = c;
    GetBotPosition() = target;
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

int AssemblySolverLayersBase::GetGreedyEstimation(int x, int y, int z) {
    size_t dummy = 0;
    return 3 * GreedyFill({x, y + 1, z}, true, dummy) -
           MoveEnergy(GetBotPosition().x - x, GetBotPosition().z - z);
}

size_t AssemblySolverLayersBase::GreedyFill(const Coordinate& c0, bool dry, size_t& count) {
    size_t result = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dz = -1; dz <= 1; ++dz) {
            Coordinate c = {c0.x + dx, c0.y - 1, c0.z + dz};
            CoordinateDifference cd = c - c0;
            if (matrix.IsInside(c) && cd.IsNearCoordinateDifferences() && NeedChange(c)) {
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
    return Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);
}

Evaluation::Result AssemblySolverLayersBase::SolveHelper(const Matrix& m, Coordinate first_and_last, Trace& output, bool levitation)
{
    AssemblySolverLayersBase solver(m, false, levitation);
    solver.SetTargetCoordinate(first_and_last);
    solver.Solve(output);
    return Evaluation::Result(solver.state.correct, solver.state.energy);
}
