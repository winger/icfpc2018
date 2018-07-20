#pragma once

#include "base.h"
#include "command.h"

class Trace
{
public:
    vector<Command> commands;

    size_t size() const { return commands.size(); }
    void ReadFromFile(const string& filename);
    void WriteToFile(const string& filename) const;
};
