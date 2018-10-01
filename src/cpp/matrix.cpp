#include "matrix.h"

#include "disjoint_set.h"

int Matrix::GetFilledVolume() const
{
    int sum = 0;
    for (int index = 0; index < data.size(); ++index)
    {
        if (Get(index))
            sum += 1;
    }
    return sum;
}

bool Matrix::IsGrounded(unordered_set<int>& ungrounded) const
{
    ungrounded.clear();
    DisjointSet ds(GetVolume() + 1);
    size_t ground = size_t(GetVolume());
    for (int x = 0; x < size; ++x)
    {
        for (int y = 0; y < size; ++y)
        {
            for (int z = 0; z < size; ++z)
            {
                if (Get(x, y, z))
                {
                    size_t index = size_t(Index(x, y, z));
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
                if (Get(x, y, z))
                {
                    size_t index = size_t(Index(x, y, z));
                    if (ds.Find(index) != ground)
                    {
                        ungrounded.insert(index);
                    }
                }
            }
        }
    }

    return ungrounded.size() == 0;
}

bool Matrix::IsGrounded() const
{
    unordered_set<int> ungrounded;
    return IsGrounded(ungrounded);
}

void Matrix::Init(int r)
{
    size = r;
    volume = size * size * size;
    data.resize(0);
    data.resize(volume, 0);
}

void Matrix::CacheYSlices() {
    ySlices = make_shared<TYSlices>();
    ySlices->resize(size);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            for (int z = 0; z < size; ++z) {
                if (Get(x, y, z)) {
                    (*ySlices)[y].emplace_back(PointXZ{x, z});
                }
            }
        }
        (*ySlices)[y].shrink_to_fit();
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
  assert(index >= 0 && index < size);
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
    return (*ySlices)[y];
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

bool Matrix::CanMove(const Coordinate& c, const CoordinateDifference& cd) const {
    CoordinateDifference step{sign(cd.dx), sign(cd.dy), sign(cd.dz)};
    unsigned l = cd.ManhattanLength();
    Coordinate bs = c;
    for (unsigned i = 1; i <= l; ++i) {
        bs += step;
        if (!IsInside(bs)) {
            return false;
        }
        if (Get(bs)) {
            return false;
        }
    }
    return true;
}

vector<CoordinateDifference> Matrix::BFS(const Coordinate& start, const Coordinate& finish) const {
    CoordinateToParent parent;
    queue<Coordinate> front;
    front.emplace(start);

    while (parent.count(finish) == 0) {
        static const vector<CoordinateDifference> DIRS = {

#define VCTS(L) \
    {-L, 0, 0}, {L, 0, 0}, {0, L, 0}, {0, -L, 0}, {0, 0, L}, { 0, 0, -L }

            VCTS(1), VCTS(2),  VCTS(3),  VCTS(4),  VCTS(5),  VCTS(6),  VCTS(7), VCTS(8),
            VCTS(9), VCTS(10), VCTS(11), VCTS(12), VCTS(13), VCTS(14), VCTS(15)

        };
        auto now = front.front();
        front.pop();
        for (const auto& d : DIRS) {
            auto cc = now + d;
            if (CanMove(now, d) && (parent.count(cc) == 0)) {
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
