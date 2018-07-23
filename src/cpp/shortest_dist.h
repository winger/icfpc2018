#pragma once

#include <vector>
#include "coordinate.h"
#include "matrix.h"

class DistMap {
public:
    DistMap(int size=1): size(size), dist(size * size * size, -1) {};

    int32_t& operator()(int x, int y, int z) {
        return dist[z + size * (y + size * x)];
    }

    int32_t& operator()(Coordinate c) {
        return dist[c.z + size * (c.y + size * c.x)];
    }
private:
    int size;
    std::vector<int32_t> dist; // -1 for unreachable
};

// Find single source shortest distances, with SMove and LMove cost == 1 and
// Void+SMove/LMove cost == 2 (if dig is turned on)
// If dig is turned on, can use Void op to dig tunnels (useful for disassembly tasks)
DistMap SingleSourceShortestDists(Matrix const& matrix, Coordinate source, bool dig);

DistMap SingleSourceShortestDists(Matrix const& matrix, vector<Coordinate> sources, bool dig);
