#include "command_line.h"
#include "evaluation.h"
#include "solver.h"

void TestTraceDecodeEncode();

int main(int argc, char* argv[])
{
    cmd.Parse(argc, argv);

    cout << "Hello!" << endl;
    // TestTraceDecodeEncode();
    // Evaluation::TestAllDfltSolution();
    Solver::SolveAll();
    return 0;
}
