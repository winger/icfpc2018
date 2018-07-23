#pragma once

#include "state.h"

struct StateSnapshot {
    Matrix matrix;
    State state;
};
using StateSnapshots = std::vector<StateSnapshot>;

class SolverBase {
protected:
    Matrix matrix;
    State state;
    bool levitation;
    Coordinate target;
    bool helper_mode;
    bool projectionGrounded{false};

    SolverBase(const Matrix& m, bool levitation);

    StateSnapshot GetSnapshot();
    void ApplySnapshot(const StateSnapshot& s);
    void SelectBestSnapshot(const StateSnapshots& s);

    void SolveInit();
    void SolveFinalize();

    void AddCommand(const Command& c) {
        state.trace.commands.emplace_back(c);
        state.Step();
    }
    Coordinate& GetBotPosition() { return state.all_bots[0].c; }
    void MoveToCoordinate(int x, int z);
    void MoveToCoordinate(int x, int y, int z, bool finalize = false);
    void MoveToCoordinate(const Coordinate& c, bool finalize = false);
};
