#include "region.h"

Region::Region(const Coordinate& c1, const Coordinate& c2)
{
    a = Coordinate{min(c1.x, c2.x), min(c1.y, c2.y), min(c1.z, c2.z)};
    b = Coordinate{max(c1.x, c2.x), max(c1.y, c2.y), max(c1.z, c2.z)};
    volume = (abs(b.x - a.x) + 1) * (abs(b.y - a.y) + 1) * (abs(b.z - a.z) + 1);
}

std::set<Coordinate> Region::Corners() const
{
    std::set<Coordinate> st;
    return std::set<Coordinate>{
        Coordinate{a.x, a.y, a.z},
        Coordinate{a.x, a.y, b.z},
        Coordinate{a.x, b.y, a.z},
        Coordinate{a.x, b.y, b.z},
        Coordinate{b.x, a.y, a.z},
        Coordinate{b.x, a.y, b.z},
        Coordinate{b.x, b.y, a.z},
        Coordinate{b.x, b.y, b.z}
    };
}
