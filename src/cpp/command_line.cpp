#include "command_line.h"

#include <cstdlib>

#include <thread>

void CommandLine::Parse(int argc, char* const argv[]) {
    int_args["threads"] = std::thread::hardware_concurrency();
    args["round"] = "full";
    int_args["levitation"] = 1;
    int_args["psort"] = 0;
    int_args["prev"] = 0;
    int_args["ah"] = 1;
    int_args["al"] = 1;
    int_args["base"] = 1;

    int index = 1;
    while (index + 1 < argc) {
        std::string arg = argv[index];
        if (arg[0] == '-') {
            arg = arg.substr(1);
        }
        args[arg] = argv[index + 1];
        int_args[arg] = atoi(argv[index + 1]);
        index += 2;
    }
}

CommandLine cmd;
