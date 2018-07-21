#pragma once

#include <unordered_map>

struct CommandLine {
    void Parse(int argc, char* const argv[]);

    std::unordered_map<std::string, std::string> args;
    std::unordered_map<std::string, int> int_args;
};

extern CommandLine cmd;
