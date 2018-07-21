#include "evaluation.h"
#include "solver.h"

void TestTraceDecodeEncode();

int main()
{
    cout << "Hello!" << endl;
    // TestTraceDecodeEncode();
    // Evaluation::TestAllDfltSolution();
    Solver::SolveAll();
    return 0;
}
