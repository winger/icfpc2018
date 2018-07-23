#pragma once

#include "base.h"
#include "coordinate.h"
#include "disjoint_set.h"
#include "interfere_check.h"
#include "matrix.h"
#include "trace.h"

class State
{
public:
    struct BotState
    {
        unsigned bid;
        Coordinate c;
        vector<unsigned> seeds;
    };

    bool correct;
    uint64_t energy;
    bool harmonics; // Low = 0, High = 1
    Matrix matrix;
    Matrix backMatrix;

    vector<BotState> all_bots;
    vector<unsigned> active_bots;
    Trace trace;
    size_t trace_pos;

    void Init(const Matrix& source, const Trace& trace);
    bool IsCorrectFinal() const;
    void Step();
    void Run();

protected:
    enum RunMode
    {
        Unknown,
        AddOnly,
        DeleteOnly,
        DeleteAndAdd,
        Hybrid
    };

    RunMode run_mode;
    unsigned filled_volume;
    bool grounded;
    DisjointSet ds;
    bool ds_rebuild_required;
    vector<int> toAdd;
    vector<int> toDelete;
    unordered_set<int> knownUngrounded;

    void Fulfill();
    void RebuildDS();

public:
    bool IsGrounded();

protected:
    bool MoveBot(BotState& bs, InterfereCheck& ic, const CoordinateDifference& cd);
};
