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

    uint8_t Get(int x, int y, int z) const { return data[z + size * (y + size * x)]; }
    uint8_t Get(const Coordinate& p) const { return Get(p.x, p.y, p.z); }
    void Fill(int x, int y, int z) { data[z + size * (y + size * x)] = 1; }
    void Fill(const Coordinate& p) { Fill(p.x, p.y, p.z); }

    void Init(int r);
    void ReadFromFile(const string& filename);
    void Print() const;
};
