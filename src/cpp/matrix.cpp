#include "matrix.h"

void Matrix::Init(int r)
{
    size = r;
    volume = size * size * size;
    data.resize(0);
    data.resize(volume, 0);
}

void Matrix::ReadFromFile(const string& filename)
{
    string full_filename = "../../problemsL/" + filename + ".mdl";
    // cout << full_filename << endl;
    ifstream file(full_filename, ios::binary);
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
