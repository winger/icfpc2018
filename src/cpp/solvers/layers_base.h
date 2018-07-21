#pragma once

#include "../solver.h"
#include "../state.h"

// TODO:
//   In SolveLayer current code use Z3 lines if possible. We can checks several solution and choose best one.
//   1. Check using SolveZ3 vs using SolveZ1.
//   2. Use DP for finding optimal combination between Z1 and Z3 strips.
//   3. Check is X-strips better than Z-strips for layer.
//   4. Check covering by crosses.
//   4.1. There are 10 different covering by crosses, it's possible to check all of them.

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
    void SolveZ1_GetRZ(int x, int y, int& z0, int& z1);
    void SolveZ1_Fill(State::BotState& bs, int x, int y, bool direction);
    void SolveZ1(int x, int y);
    void SolveZ3_GetRZ(int x, int y, int& z0, int& z1);
    void SolveZ3_Fill(State::BotState& bs, int x, int y, bool direction);
    void SolveZ3(int x, int y);
    void SolveLayer(int y);
    void SolveFinalize();
    void Solve(Trace& output);

public:
    static uint64_t Solve(const Matrix& m, Trace& output);
};
