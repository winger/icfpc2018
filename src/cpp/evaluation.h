#pragma once

#include "matrix.h"
#include "trace.h"

class Evaluation
{
public:
    struct Result
    {
        bool correct;
        uint64_t energy;
        int r;

        Result(bool _correct = false, uint64_t _energy = 0, int _r = 0) : correct(_correct), energy(_energy), r(_r) {}

        bool operator<(const Result& r) const { return correct && (!r.correct || (energy < r.energy)); }
    };

    static Result Evaluate(const Matrix& source, const Matrix& target, const Trace& t);
};
