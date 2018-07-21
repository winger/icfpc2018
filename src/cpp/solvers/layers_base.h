#pragma once

#include "../solver.h"
#include "../state.h"

class SolverLayersBase
{
protected:
    Matrix matrix;
    State state;

    SolverLayersBase(const Matrix& m);

    void AddCommand(const Command& c) { state.trace.commands.push_back(c); state.Step(); }
    void MoveToCoordinate(State::BotState& bs, int x, int z);
    void MoveToCoordinate(State::BotState& bs, int x, int y, int z);

    void SolveInit();
    void SolveZ1(int x, int y);
    void SolveLayer(int y);
    void SolveFinalize();
    void Solve(Trace& output);

public:
    static uint64_t Solve(const Matrix& m, Trace& output);
};
