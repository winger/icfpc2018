#include "evaluation.h"

#include "state.h"

Evaluation::Result Evaluation::Evaluate(const Matrix& source, const Matrix& target, const Trace& t)
{
    State s;
    s.Init(source, t);
    s.Run();
    bool correct = (s.IsCorrectFinal() && (s.matrix == target));
    return Result{correct, s.energy};
}

uint64_t Evaluation::CheckSolution(const Matrix& model, const Trace& t)
{
    State s;
    s.Init(Matrix(model.GetR()), t);
    s.Run();
    return (s.IsCorrectFinal() && (s.matrix == model)) ? s.energy : 0;
}

void Evaluation::TestDfltSolution(unsigned model_index)
{
    string si = to_string(1000 + model_index).substr(1);
    Matrix model;
    model.ReadFromFile("LA" + si + "_tgt");
    Trace trace;
    trace.ReadFromFile("dfltTracesL/LA" + si + ".nbt");
    uint64_t energy = CheckSolution(model, trace);
    cout << "Test " << si << ": " << energy << endl;
}

void Evaluation::TestAllDfltSolution()
{
    for (unsigned i = 1; i <= 186; ++i)
        TestDfltSolution(i);
}
