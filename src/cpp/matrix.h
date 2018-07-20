#pragma once

#include "base.h"
#include "coordinate.h"

class Matrix
{
protected:
    int size;
    int volume;
    vector<uint8_t> data;

public:
    int GetR() const { return size; }
    int GetVolume() const { return volume; }

    bool IsInside(int x, int y, int z) const { return (0 <= x) && (x < size) && (0 <= y) && (y < size) && (0 <= z) && (z < size); }
    bool IsInside(const Coordinate& c) const { return IsInside(c.x, c.y, c.z); }
    uint8_t Get(int x, int y, int z) const { return data[z + size * (y + size * x)]; }
    uint8_t Get(const Coordinate& c) const { return Get(c.x, c.y, c.z); }
    void Fill(int x, int y, int z) { data[z + size * (y + size * x)] = 1; }
    void Fill(const Coordinate& c) { Fill(c.x, c.y, c.z); }

    void Init(int r);
    void ReadFromFile(const string& filename);
    void Print() const;
};
