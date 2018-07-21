#include "constants.h"
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

    auto round = cmd.args["round"];

    if (round == "lightning") {
        TaskConsts::N_BOTS = 20;
    }

    cout << "Mode: " << mode << endl;
    if (mode == "solve") {
        Solver::SolveAll(cmd.args["round"]);
    } else if (mode == "grounder") {
        Grounder::CheckAll();
    } else if (mode == "check") {
        Solver::CheckAll(round);
    } else if (mode == "merge") {
        Solver::MergeWithSubmit(round);
    } else {
        assert(false);
    }
    // TestTraceDecodeEncode();
    // Evaluation::TestAllDfltSolution();
    return 0;
}
