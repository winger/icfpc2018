#pragma once

#include "base.h"
#include "coordinate.h"
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

    uint64_t energy;
    bool harmonics; // Low = 0, High = 1
    Matrix matrix;
    vector<unsigned> active_bots;
    vector<BotState> all_bots;
    Trace trace;
    size_t trace_pos;
};
