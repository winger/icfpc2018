#pragma once

#include "matrix.h"
#include "trace.h"

class Evaluation
{
public:
    struct Result
    {
        bool correct = false;
        uint64_t energy;
        int r;

        bool operator<(const Result& r) const { return correct && (!r.correct || (energy < r.energy)); }
    };

    static Result Evaluate(const Matrix& source, const Matrix& target, const Trace& t);
};
