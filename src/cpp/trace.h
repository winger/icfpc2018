#pragma once

#include "base.h"
#include "command.h"

class Trace
{
public:
    Trace();

    vector<Command> commands;
    // for debug output
    std::string tag;

    size_t size() const { return commands.size(); }
    void ReadFromFile(const string& filename);
    bool TryReadFromFile(const string& filename);

    void WriteToFile(const string& filename) const;

    static Trace Cat(const Trace& a, const Trace& b);
    void Done();
    int Duration() const;
    void OverrideDuration(int duration);

private:
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> finish;
    int duration{};
};

ostream& operator<<(ostream& s, const Trace& t);
