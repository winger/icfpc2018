#include "problem.h"

#include "command_line.h"

std::string Problem::GetType() const {
    assert(assembly + disassembly + reassembly == 1);
    if (assembly) {
        return "A";
    }
    if (disassembly) {
        return "D";
    }
    if (reassembly) {
        return "R";
    }
    assert(false);
}

std::string Problem::GetSI() const { return to_string(1000 + index).substr(1); }

std::string Problem::GetPrefix() const {
    if (round == "L") {
        return "../../lightning/";
    } else if (round == "F") {
        if (cmd.args["round"] == "postfull") {
            return "../../postfull/";
        } else {
            return "../../";
        }
    } else {
        std::cerr << round << std::endl;
        assert(false);
    }
}

std::string Problem::GetSource() const {
    auto filename = round + GetType() + GetSI() + "_src";

    return GetPrefix() + "problems" + round + "/" + filename + ".mdl";
}

std::string Problem::GetTarget() const {
    auto filename = round + GetType() + GetSI() + "_tgt";

    return GetPrefix() + "problems" + round + "/" + filename + ".mdl";
}

std::string Problem::GetProxy() const { return GetPrefix() + "proxyTraces" + round + "/" + round + GetType() + GetSI() + ".nbt"; }

std::string Problem::GetDefaultTrace() const { return GetPrefix() + "dfltTraces" + round + "/" + round + GetType() + GetSI() + ".nbt"; }

std::string Problem::GetEnergyInfo() const { return GetPrefix() + "tracesEnergy" + round + "/" + round + GetType() + GetSI() + ".txt"; }

std::string Problem::GetOutput() const { return GetPrefix() + "cppTraces" + round + "/" + round + GetType() + GetSI() + ".nbt"; }

std::string Problem::GetSubmitEnergyInfo() const {
    return GetPrefix() + "submitEnergy" + round + "/" + round + GetType() + GetSI() + ".txt";
}
std::string Problem::GetSubmitOutput() const {
    return GetPrefix() + "submitTraces" + round + "/" + round + GetType() + GetSI() + ".nbt";
}

std::string Problem::Name() const {
    return GetType() + GetSI();
}

std::string Problem::GetLogFile() const {
    return GetPrefix() + "logs/" + round + GetType() + GetSI() + ".txt";
}

std::string Problem::GetMetadataFile() const {
    return GetPrefix() + "metadata/" + round + GetType() + GetSI() + ".txt";
}
