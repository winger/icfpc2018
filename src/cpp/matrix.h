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
    uint8_t Get(int x, int y, int z) const { return data[Index(x, y, z)]; }
    uint8_t Get(const Coordinate& c) const { return Get(c.x, c.y, c.z); }
    void Fill(int x, int y, int z) { data[z + size * (y + size * x)] = 1; }
    void Fill(const Coordinate& c) { Fill(c.x, c.y, c.z); }
    void Erase(int x, int y, int z) { data[z + size * (y + size * x)] = 0; }
    void Erase(const Coordinate& c) { Erase(c.x, c.y, c.z); }
    int Index(int x, int y, int z) const { return z + size * (y + size * x); }
    std::vector<int> Reindex(int index) const;

    uint32_t FullNum() const { return std::count(data.begin(), data.end(), 1); }

    bool IsGrounded() const;

    Matrix(int r = 0) { Init(r); }
    void Init(int r);
    void ReadFromFile(const string& filename);
    void Print() const;

    bool operator==(const Matrix& m) const { return data == m.data; }
};
