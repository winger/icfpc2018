#pragma once

#include "base.h"
#include "coordinate.h"
#include "hash.h"

class Region
{
protected:
    int volume;

public:
    Coordinate a, b;

    Region(const Coordinate& c1, const Coordinate& c2);

    int GetVolume() const { return volume; }
    unsigned Dimensions() const { return (a.x != b.x ? 1 : 0) + (a.y != b.y ? 1 : 0) + (a.z != b.z ? 1 : 0); }
    std::set<Coordinate> Corners() const;

    bool operator==(const Region& r) const { return a == r.a && b == r.b; }
};

inline bool operator< (const Region& lhs, const Region& rhs)
{
    return std::tie(lhs.a, lhs.b) < std::tie(lhs.a, lhs.b);
};

namespace std {

template<>
struct hash<Region>
{
    size_t operator()(const Region& r) const
    {
        hash<Coordinate> h;
        return hash_combine(h(r.a), h(r.b));
    }
};

} // std