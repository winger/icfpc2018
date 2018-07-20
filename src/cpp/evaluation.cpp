#include "evaluation.h"

#include "state.h"

uint64_t Evaluation::CheckSolution(const Matrix& model, const Trace& t)
{
    State s;
    s.Init(model.GetR(), t);
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
