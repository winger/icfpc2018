#pragma once

#include "base.h"
#include "command.h"

class Trace
{
public:
    vector<Command> commands;

    size_t size() const { return commands.size(); }
    void ReadFromFile(const string& filename);
    bool TryReadFromFile(const string& filename);

    void WriteToFile(const string& filename) const;

    static Trace Cat(const Trace& a, const Trace& b);
};
