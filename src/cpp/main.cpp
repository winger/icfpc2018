#include "command_line.h"
#include "evaluation.h"
#include "solver.h"
#include "grounder.h"

void TestTraceDecodeEncode();

int main(int argc, char* argv[])
{
    cmd.Parse(argc, argv);

    auto mode = cmd.args["mode"];
    if (mode.empty()) {
        mode = "solve";
    }

    cout << "Mode: " << mode << endl;
    if (mode == "solve") {
        Solver::SolveAll(cmd.args["round"]);
    } else if (mode == "grounder") {
        Grounder::CheckAll();
    } else if (mode == "check") {
        Solver::CheckAll(cmd.args["round"]);
    } else {
        assert(false);
    }
    // TestTraceDecodeEncode();
    // Evaluation::TestAllDfltSolution();
    return 0;
}
