#pragma once

#include <map>

struct CommandLine {
    void Parse(int argc, char* const argv[]);

    std::map<std::string, std::string> args;
    std::map<std::string, int> int_args;
};

extern CommandLine cmd;
