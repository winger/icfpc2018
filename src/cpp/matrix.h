#pragma once

#include <memory>

#include "base.h"
#include "coordinate.h"
#include "coordinate_difference.h"

struct PointXZ {
    int16_t x;
    int16_t z;
};

using CoordinateSet = set<Coordinate>;

class Matrix
{
protected:
    int size;
    int volume;
    vector<uint8_t> data;
    using TYSlices = vector<vector<PointXZ>>;
    shared_ptr<TYSlices> ySlices;

public:
    int GetR() const { return size; }
    int GetVolume() const { return volume; }

    bool IsInside(int x, int y, int z) const { return (0 <= x) && (x < size) && (0 <= y) && (y < size) && (0 <= z) && (z < size); }
    bool IsInside(const Coordinate& c) const { return IsInside(c.x, c.y, c.z); }
    uint8_t Get(int index) const { return data[index]; }
    uint8_t Get(int x, int y, int z) const { return data[Index(x, y, z)]; }
    uint8_t Get(const Coordinate& c) const { return Get(c.x, c.y, c.z); }
    void Fill(int index) { data[index] = 1; }
    void Fill(int x, int y, int z) { data[Index(x, y, z)] = 1; }
    void Fill(const Coordinate& c) { Fill(c.x, c.y, c.z); }
    void Erase(int index) { data[index] = 0; }
    void Erase(int x, int y, int z) { data[Index(x, y, z)] = 0; }
    void Erase(const Coordinate& c) { Erase(c.x, c.y, c.z); }
    int Index(int x, int y, int z) const { return z + size * (y + size * x); }
    std::vector<int> Reindex(int index) const;

    // Block copy/clear. x1, y1, z1 are not included!
    void CopyBlock(const Matrix& source, int x0, int x1, int y0, int y1, int z0, int z1);
    void EraseBlock(int x0, int x1, int y0, int y1, int z0, int z1);

    uint32_t FullNum() const { return std::count(data.begin(), data.end(), 1); }

    bool IsGrounded() const;

    Matrix(int r = 0) { Init(r); }
    void Init(int r);
    void ReadFromFile(const string& filename);
    void Print() const;

    void CacheYSlices();
    const vector<PointXZ>& YSlices(int y) const;

    bool operator==(const Matrix& m) const { return data == m.data; }

    void DFS(const Coordinate& c, CoordinateSet& cs) const;
    bool CanMove(const Coordinate& c, const CoordinateDifference& cd) const;
    vector<CoordinateDifference> BFS(const Coordinate& start, const Coordinate& finish) const;
};

std::ostream& operator<<(std::ostream& s, const Matrix& m);
