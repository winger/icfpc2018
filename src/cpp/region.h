#pragma once

#include "base.h"
#include "coordinate.h"
#include "hash.h"

class Region
{
protected:
    Coordinate a, b;
    int volume;

public:
    Region(const Coordinate& c1, const Coordinate& c2);

    int GetVolume() const { return volume; }
    unsigned Dimensions() const { return (a.x != b.x ? 1 : 0) + (a.y != b.y ? 1 : 0) + (a.z != b.z ? 1 : 0); }
    Coordinate GetA() const { return a; }
    Coordinate GetB() const { return b; }
    std::vector<Coordinate> Corners() const;

    bool operator==(const Region& r) const { return a == r.a && b == r.b; }
};

namespace std {

template<>
struct hash<Region>
{
    size_t operator()(const Region& r) const
    {
        hash<Coordinate> h;
        return hash_combine(h(r.GetA()), h(r.GetB()));
    }
};

} // std