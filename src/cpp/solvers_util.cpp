#include "solvers_util.h"

SolverBase::SolverBase(const Matrix& m, bool l) : matrix(m), levitation(l) {
    state.Init(m.GetR(), Trace());
    targetC = {0, 0, 0};
}

void SolverBase::SolveInit() {
    if (!helper_mode) {
        if (!projectionGrounded) {
            if (levitation) {
                AddCommand(Command(Command::Flip));
            }
        }
    }
}

void SolverBase::SolveFinalize() {
    if (helper_mode) {
        MoveToCoordinate(targetC, true);
    } else {
        if (!projectionGrounded) {
            if (levitation) {
                AddCommand(Command(Command::Flip));
            }
        }
        MoveToCoordinate(targetC, true);
        AddCommand(Command(Command::Halt));
    }
}

void SolverBase::MoveToCoordinate(int x, int z)
{
    assert(x >= 0 && x < matrix.GetR());
    assert(z >= 0 && z < matrix.GetR());
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

void SolverBase::MoveToCoordinate(int x, int y, int z, bool finalize)
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

StateSnapshot SolverBase::GetSnapshot() {
    return {state};
}

void SolverBase::ApplySnapshot(const StateSnapshot& s) {
    state = s.state;
}

void SolverBase::SelectBestSnapshot(const StateSnapshots& s) {
    size_t best_energy = s[0].state.energy;
    size_t best_index = 0;
    for (size_t i = 1; i < s.size(); ++i) {
        assert(s[i].state.matrix == s[0].state.matrix);
        if (s[i].state.energy < best_energy) {
            best_energy = s[i].state.energy;
            best_index = i;
        }
    }
    ApplySnapshot(s[best_index]);
}

void SolverBase::MoveToCoordinate(const Coordinate& c, bool finalize) {
    MoveToCoordinate(c.x, c.y, c.z, finalize);
}
