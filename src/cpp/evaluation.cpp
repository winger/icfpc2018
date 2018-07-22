#include "evaluation.h"

#include "state.h"

Evaluation::Result Evaluation::Evaluate(const Matrix& source, const Matrix& target, const Trace& t)
{
    State s;
    s.Init(source, t);
    s.Run();
    bool correct = (s.IsCorrectFinal() && (s.matrix == target));
    return Result(correct, s.energy, source.GetR());
}
