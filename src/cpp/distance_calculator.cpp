#include "distance_calculator.h"

#include "state.h"

struct MoveEnergyCalculator {
    static constexpr size_t MAX = 300;

    MoveEnergyCalculator() {
        vector<int16_t> dummy1(MAX);
        distances.resize(MAX, dummy1);

        for (size_t i = 0; i < MAX; ++i) {
            for (size_t j = 0; j < MAX; ++j) {
                distances[i][j] = MoveEnergy(i, j);
            }
        }
    }

    int16_t MoveEnergy(int x, int z) const {
        int16_t result = 0;

        Coordinate bc = {0, 0, 0};
        Command c(Command::SMove);
        for (; abs(x - bc.x) > 5;) {
            c.cd1 = {max(-15, min(x - bc.x, 15)), 0, 0};
            result += c.Energy();
            bc += c.cd1;
        }
        for (; abs(z - bc.z) > 5;) {
            c.cd1 = {0, 0, max(-15, min(z - bc.z, 15))};
            result += c.Energy();
            bc += c.cd1;
        }
        if (bc.x == x) {
            if (bc.z == z) {
                // Already here
            } else {
                c.cd1 = {0, 0, z - bc.z};
                result += c.Energy();
                bc += c.cd1;
            }
        } else {
            if (bc.z == z) {
                c.cd1 = {x - bc.x, 0, 0};
                result += c.Energy();
                bc += c.cd1;
            } else {
                c.type = Command::LMove;
                c.cd1 = {x - bc.x, 0, 0};
                c.cd2 = {0, 0, z - bc.z};
                result += c.Energy();
                bc += c.cd1;
                bc += c.cd2;
            }
        }

        return result;
    }

    int16_t Get(int x, int z) const {
        assert(x >= 0 && x < MAX);
        assert(z >= 0 && z < MAX);
        return distances[x][z];
    }

    vector<vector<int16_t>> distances;
};

struct MoveEnergyCalculator3D {
    static constexpr size_t MAX = 50;

    MoveEnergyCalculator3D() {
        vector<int> dummy1(MAX);
        vector<vector<int>> dummy2(MAX, dummy1);
        distances.resize(MAX, dummy2);

        for (size_t i = 0; i < MAX; ++i) {
            for (size_t j = 0; j < MAX; ++j) {
                for (size_t k = 0; k < MAX; ++k) {
                    distances[i][j][k] = MoveEnergy(i, j, k);
                }
            }
        }
    }

    int MoveEnergy(int x, int y, int z) const {
        return 8*(abs(x) + abs(y) + abs(z));
    }

    int Get(int x, int y, int z) const {
        if (x < 0 || x >= MAX) {
            return INF_DISTANCE;
        }
        if (y < 0 || y >= MAX) {
            return INF_DISTANCE;
        }
        if (z < 0 || z >= MAX) {
            return INF_DISTANCE;
        }
        return distances[x][y][z];
    }

    vector<vector<vector<int>>> distances;
};

int MoveEnergy(int x, int z) {
    static const MoveEnergyCalculator clc;
    return clc.Get(abs(x), abs(z));
}

int MoveEnergy(const Coordinate& a, const Coordinate& b) {
    static const MoveEnergyCalculator3D clc;
    return clc.Get(abs(a.x - b.x), abs(a.y - b.y), abs(a.z - b.z));
}
