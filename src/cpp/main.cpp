#include "command_line.h"
#include "evaluation.h"
#include "solver.h"
#include "grounder.h"

void TestTraceDecodeEncode();

int main(int argc, char* argv[])
{
    cmd.Parse(argc, argv);

    cout << "Hello!" << endl;
    // TestTraceDecodeEncode();
    // Evaluation::TestAllDfltSolution();
    // Grounder::CheckAll();
    Solver::SolveAll();
    return 0;
}
