#include "trace.h"

bool Trace::TryReadFromFile(const string& filename) {
    if (!FileExists(filename)) {
        return false;
    }
    ReadFromFile(filename);
    return true;
}

void Trace::ReadFromFile(const string& filename) {
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
    for (Command t; pos < size;) {
        t.Decode(data, pos);
        commands.push_back(t);
    }
    assert(pos == size);
    tag = filename;
    // cout << "Total commands = " << commands.size() << endl;
}

void Trace::WriteToFile(const string& filename) const {
    vector<uint8_t> data;
    for (const Command& c : commands) c.Encode(data);
    ofstream file(filename, ios::binary);
    assert(file.is_open());
    file.write(reinterpret_cast<char*>(data.data()), data.size());
    file.close();
}

Trace Trace::Cat(const Trace& a, const Trace& b) {
    Trace result = a;
    if (!result.commands.empty()) {
        assert(result.commands.back().type == Command::Halt);
        result.commands.pop_back();
    }
    result.commands.insert(result.commands.end(), b.commands.begin(), b.commands.end());
    result.tag = a.tag + ", " + b.tag;
    return result;
}

ostream& operator<<(ostream& s, const Trace& t) {
    s << "[" << endl;
    for (const auto& c : t.commands) {
        s << "\t" << c << endl;
    }
    s << "]" << endl;
    return s;
}
