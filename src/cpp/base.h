#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
// #include <utility>

#include <sys/stat.h>
#include "unistd.h"

using namespace std;

inline int sign(int x) { return (x < 0) ? -1 : (x > 0) ? 1 : 0; }

class StopException : public std::exception {
    using std::exception::exception;
};

class UnsupportedException : public std::exception {
    using std::exception::exception;
};


inline bool FileExists(const std::string& filename) {
    struct stat buffer;
    return stat(filename.c_str(), &buffer) == 0;
}
