#pragma once

#include "base.h"

struct CheckResult {
    bool ok;
    unsigned score;
};

struct Problem {
    size_t index;
    std::string si;
    std::string round;
    bool assembly{};
    bool disassembly{};
    bool reassembly{};
    std::string GetPrefix() const;
    std::string GetType() const;
    std::string GetSI() const;
    std::string GetSource() const;
    std::string GetTarget() const;
    std::string GetProxy() const;
    std::string GetDefaultTrace() const;
    std::string GetEnergyInfo() const;
    std::string GetOutput() const;
    std::string GetLogFile() const;

    std::string GetSubmitOutput() const;
    std::string GetSubmitEnergyInfo() const;

    std::string Name() const;
};

using Problems = std::vector<Problem>;
