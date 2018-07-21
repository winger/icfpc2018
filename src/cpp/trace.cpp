#include "trace.h"

bool exist(const std::string& name)
{
    ifstream file(name);
    if(!file)            // If the file was not found, then file is 0, i.e. !file=1 or true.
        return false;    // The file was not found.
    else                 // If the file was found, then file is non-0.
        return true;     // The file was found.
}

bool Trace::TryReadFromFile(const string& filename) {
  if (!exist(filename)) {
    return false;
  }
  ReadFromFile(filename);
  return true;
}


void Trace::ReadFromFile(const string& filename)
{
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Trace " << filename << " not found" << endl;
    }
    assert(file.is_open());
    // cout << "File is open.";
    file.seekg(0, ios::end);
    size_t size = file.tellg();
    file.seekg(0, ios::beg);
    // cout << " Size = " << size << endl;
    vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(&(data[0])), size);
    file.close();

    commands.clear();
    size_t pos = 0;
    for (Command t; pos < size; )
    {
        t.Decode(data, pos);
        commands.push_back(t);
    }
    assert(pos == size);
    // cout << "Total commands = " << commands.size() << endl;
}

void Trace::WriteToFile(const string& filename) const
{
    vector<uint8_t> data;
    for (const Command& c : commands)
        c.Encode(data);
    ofstream file(filename, ios::binary);
    file.write(reinterpret_cast<char*>(data.data()), data.size());
    file.close();
}
