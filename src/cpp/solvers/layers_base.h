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

    bool helper_mode;
    Coordinate target;

    SolverLayersBase(const Matrix& m);

    void SetTargetCoordinate(const Coordinate& c);

    void AddCommand(const Command& c) { state.trace.commands.push_back(c); state.Step(); }
    Coordinate& GetBotPosition() { return state.all_bots[0].c; }
    void MoveToCoordinate(int x, int z);
    void MoveToCoordinate(int x, int y, int z, bool finalize = false);

    void SolveInit();
    void SolveZ1_GetRZ(int x, int y, int& z0, int& z1);
    void SolveZ1_Fill(int x, int y, bool direction);
    void SolveZ1(int x, int y);
    void SolveZ3_GetRZ(int x, int y, int& z0, int& z1);
    void SolveZ3_Fill(int x, int y, bool direction);
    void SolveZ3(int x, int y);
    void SolveLayer(int y);
    void SolveFinalize();
    void Solve(Trace& output);

public:
    static uint64_t Solve(const Matrix& m, Trace& output);
    static uint64_t SolveHelper(const Matrix& m, Coordinate first_and_last, Trace& output);
};
