#pragma once

#include "base.h"
#include "coordinate.h"

struct CoordinateDifference
{
    int dx, dy, dz;

    unsigned ManhattanLength() const { return abs(dx) + abs(dy) + abs(dz); }
    unsigned ChessboardLength() const { return max(max(abs(dx), abs(dy)), abs(dz)); }
    unsigned CoordinateChanged() const { return (dx ? 1 : 0) + (dy ? 1 : 0) + (dz ? 1 : 0); }
    bool IsLinearCoordinateDifferences() const { return CoordinateChanged() == 1; }
    bool IsShortLinearCoordinateDifferences() const { return IsLinearCoordinateDifferences() && (ManhattanLength() <= 5); }
    bool IsLongLinearCoordinateDifferences() const { return IsLinearCoordinateDifferences() && (ManhattanLength() <= 15); }
    bool IsNearCoordinateDifferences() const { return (ChessboardLength() == 1) && (ManhattanLength() <= 2); }
};

inline Coordinate operator+(const Coordinate& c, const CoordinateDifference& cd) { return Coordinate{c.x + cd.dx, c.y + cd.dy, c.z + cd.dz}; }
