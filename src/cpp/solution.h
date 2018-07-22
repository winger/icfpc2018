#pragma once

#include "base.h"
#include "evaluation.h"
#include "trace.h"

struct Solution
{
    Trace trace;
    bool correct;

    uint64_t energy;
    uint64_t dflt_energy;
    unsigned score;
    unsigned max_score;

    void Set(const Evaluation::Result& solution, const Evaluation::Result& dflt);
};
