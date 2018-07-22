#include "solution.h"

void Solution::Set(const Evaluation::Result& solution, const Evaluation::Result& dflt)
{
    assert(dflt.correct);
    correct = solution.correct;
    energy = solution.energy;
    dflt_energy = dflt.energy;
    double performance = correct ? (1.0 - double(energy) / double(dflt_energy)) : 0.;
    score = unsigned(1000.0 * performance * unsigned(log(dflt.r) / log(2)));
    max_score = unsigned(1000.0 * unsigned(log(dflt.r) / log(2)));
}

bool Solution::operator<(const Solution& s) const {
    assert(correct);
    assert(s.correct);
    return energy < s.energy;
}
