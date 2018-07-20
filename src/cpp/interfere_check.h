#pragma once

#include "base.h"
#include "coordinate.h"

class InterfereCheck
{
protected:
    bool valid;
    unordered_set<Coordinate> s;

public:
    InterfereCheck() { Reset(); }
    void Reset() { valid = true; s.clear(); }
    bool IsValid() const { return valid; }
    void AddCoordinate(const Coordinate& c)
    {
        valid = valid && (s.find(c) == s.end());
        s.insert(c);
    }
};
