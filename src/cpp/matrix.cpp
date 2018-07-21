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

void Matrix::ReadFromFile(const string& filename)
{
    // cout << full_filename << endl;
    ifstream file(filename, ios::binary);
    assert(file.is_open());
    // cout << "File is open.";
    char c;
    file.read(&c, 1);
    Init(uint8_t(c));
    // cout << " Size = " << size << endl;
    volume = size * size * size;
    unsigned volume8 = (volume + 7) / 8;
    vector<uint8_t> data_bool(volume8);
    file.read(reinterpret_cast<char*>(&(data_bool[0])), volume8);
    file.close();
    for (unsigned i = 0; i < volume; ++i)
    {
        data[i] = (data_bool[i / 8] >> (i % 8)) & 1;
    }
}

void Matrix::Print() const
{
    for (int y = size-1; y >= 0; --y)
    {
        for (int z = size-1; z >= 0; --z)
        {
            for (int x = 0; x < size; ++x)
                cout << (Get(x, y, z) ? "#" : ".");
            cout << endl;
        }
        cout << endl;
    }
}

std::vector<int> Matrix::Reindex(int index) const {
  int z = index % size;
  index /= size;
  int y = index % size;
  index /= size;
  int x = index % size;
  return std::vector<int>{x, y, z};
}
