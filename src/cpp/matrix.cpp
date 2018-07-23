#include "matrix.h"

#include "disjoint_set.h"

bool Matrix::IsGrounded() const
{
    DisjointSet ds(GetVolume() + 1);
    size_t ground = size_t(GetVolume());
    for (int x = 0; x < size; ++x)
    {
        for (int y = 0; y < size; ++y)
        {
            for (int z = 0; z < size; ++z)
            {
                size_t index = size_t(Index(x, y, z));
                if (Get(x, y, z))
                {
                    if (y == 0)
                        ds.Union(ground, index);
                    else if (Get(x, y-1, z))
                        ds.Union(index, size_t(Index(x, y-1, z)));
                    if ((x > 0) && Get(x-1, y, z))
                        ds.Union(index, size_t(Index(x-1, y, z)));
                    if ((z > 0) && Get(x, y, z-1))
                        ds.Union(index, size_t(Index(x, y, z-1)));
                }
            }
        }
    }

    ground = ds.Find(ground);
    for (int x = 0; x < size; ++x)
    {
        for (int y = 0; y < size; ++y)
        {
            for (int z = 0; z < size; ++z)
            {
                size_t index = size_t(Index(x, y, z));
                if (Get(x, y, z))
                {
                    if (ds.Find(index) != ground)
                        return false;
                }
            }
        }
    }

    return true;
}

void Matrix::Init(int r)
{
    size = r;
    volume = size * size * size;
    data.resize(0);
    data.resize(volume, 0);
}

void Matrix::CacheYSlices() {
    ySlices.resize(size);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            for (int z = 0; z < size; ++z) {
                if (Get(x, y, z)) {
                    ySlices[y].emplace_back(PointXZ{x, z});
                }
            }
        }
        ySlices.shrink_to_fit();
    }
}

void Matrix::ReadFromFile(const string& filename) {
    // cout << full_filename << endl;
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Could not read:" << filename << endl;
    }
    assert(file.is_open());
    // cout << "File is open.";
    char c;
    file.read(&c, 1);
    Init(uint8_t(c));
    // cout << " Size = " << size << endl;
    volume = size * size * size;
    unsigned volume8 = (volume + 7) / 8;
    vector<uint8_t> data_bool(volume8);
    file.read(reinterpret_cast<char*>(data_bool.data()), volume8);
    file.close();
    for (unsigned i = 0; i < volume; ++i) {
        data[i] = (data_bool[i / 8] >> (i % 8)) & 1;
    }

    CacheYSlices();
}

std::ostream& operator<<(std::ostream& s, const Matrix& m) {
    for (int y = m.GetR() - 1; y >= 0; --y) {
        cout << "y = " << y << endl;
        for (int z = m.GetR() - 1; z >= 0; --z) {
            for (int x = 0; x < m.GetR(); ++x) {
                s << (m.Get(x, y, z) ? "#" : ".");
            }
            s << endl;
        }
        s << endl;
    }
    return s;
}

std::vector<int> Matrix::Reindex(int index) const {
  int z = index % size;
  index /= size;
  int y = index % size;
  index /= size;
  int x = index % size;
  return std::vector<int>{x, y, z};
}

void Matrix::CopyBlock(const Matrix& source, int x0, int x1, int y0, int y1, int z0, int z1)
{
    assert(size == source.size);
    for (int x = x0; x < x1; ++x)
    {
        for (int y = y0; y < y1; ++y)
        {
            for (int z = z0; z < z1; ++z)
            {
                int index = Index(x, y, z);
                data[index] = source.data[index];
            }
        }
    }
}

void Matrix::EraseBlock(int x0, int x1, int y0, int y1, int z0, int z1)
{
    for (int x = x0; x < x1; ++x)
    {
        for (int y = y0; y < y1; ++y)
        {
            for (int z = z0; z < z1; ++z)
            {
                int index = Index(x, y, z);
                data[index] = 0;
            }
        }
    }
}

const vector<PointXZ>& Matrix::YSlices(int y) const {
    assert(y >= 0 && y < size);
    return ySlices[y];
}

void Matrix::DFS(const Coordinate& c, CoordinateSet& cs) const {
    if (!IsInside(c)) {
        return;
    }

    if (Get(c)) {
        return;
    }

    if (cs.count(c)) {
        return;
    }

    cs.insert(c);

    DFS({c.x - 1, c.y, c.z}, cs);
    DFS({c.x + 1, c.y, c.z}, cs);
    DFS({c.x, c.y - 1, c.z}, cs);
    DFS({c.x, c.y + 1, c.z}, cs);
    DFS({c.x, c.y, c.z - 1}, cs);
    DFS({c.x, c.y, c.z + 1}, cs);
}

using CoordinateToParent = map<Coordinate, Coordinate>;

vector<CoordinateDifference> Matrix::BFS(const Coordinate& start, const Coordinate& finish) const {
    CoordinateToParent parent;
    queue<Coordinate> front;
    front.emplace(start);

    while (parent.count(finish) == 0) {
        static const vector<CoordinateDifference> DIRS = {{-1, 0, 0}, {1, 0, 0}, {0, 1, 0},
                                                          {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
        auto now = front.front();
        front.pop();
        for (const auto& d : DIRS) {
            auto cc = now + d;
            if (IsInside(cc) && !Get(cc) && (parent.count(cc) == 0)) {
                parent[cc] = now;
                front.emplace(cc);
            }
        }
    }

    vector<CoordinateDifference> result;
    auto now = finish;
    while (now != start) {
        auto p = parent[now];
        result.emplace_back(now - p);
        now = p;
    }
    reverse(result.begin(), result.end());
    return result;
}
