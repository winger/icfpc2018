#pragma once

#include "hash.h"

struct Coordinate
{
    int16_t x, y, z;

    bool operator==(const Coordinate& c) const { return (x == c.x) && (y == c.y) && (z == c.z); }
    bool operator!=(const Coordinate& c) const { return (x != c.x) || (y != c.y) || (z != c.z); }
};

ostream& operator<<(ostream& s, const Coordinate& c);


inline bool operator< (const Coordinate& lhs, const Coordinate& rhs)
{
    return std::tie(lhs.x, lhs.y, lhs.z) < std::tie(rhs.x, rhs.y, rhs.z);
};

namespace std {

template<>
struct hash<Coordinate>
{
    size_t operator()(const Coordinate& c) const
    {
        hash<int> h;
        return hash_combine(hash_combine(h(c.x), h(c.y)), h(c.z));
    }
};

} // std
