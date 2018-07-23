#include "3d_demolition.h"

constexpr bool debug = false, debug_3d_demolition = true;

Solver3D_Demolition::Solver3D_Demolition(const Matrix& target): matrix(target) {}

void Solver3D_Demolition::Solve() {
    state.Init(matrix.GetR(), Trace());
    assert(matrix.GetR() >= 7);
    AllFission();
    Flip();

    InitDistToBoundary();
    while(DoDemolish());

    Flip();
    AllFusion();
    ExecuteCommands({{Command::Type::Halt}});
}

void Solver3D_Demolition::InitDistToBoundary() {
    vector<Coordinate> boundary;
    for (int u = 0; u < matrix.GetR(); ++u) {
        for (int v = 0; v < matrix.GetR(); ++v) {
            boundary.push_back(Coordinate{u, v, 0});
            boundary.push_back(Coordinate{u, v, matrix.GetR() - 1});
            boundary.push_back(Coordinate{0, u, v});
            boundary.push_back(Coordinate{matrix.GetR() - 1, u, v});
            boundary.push_back(Coordinate{u, matrix.GetR() - 1, v});
        }
    }
    distToBoundary = SingleSourceShortestDists(matrix, boundary, true);
}

// TODO: proper solution instead of greedy
int assignmentCost(vector<vector<int>> const& ds, vector<int>& dx, vector<int>& dy) {
    if (ds.size() == 0) {
        return 0;
    }
    dx.assign(ds.size(), -1);
    dy.assign(ds[0].size(), -1);

    int sum = 0;
    while (true) {
        int bx = -1, by = -1;
        for (int x = 0; x < ds.size(); ++x) {
            if (dx[x] != -1) {
                continue;
            }
            for (int y = 0; y < ds[0].size(); ++y) {
                if (dy[y] != -1) {
                    continue;
                }
                if (bx == -1 || ds[x][y] < ds[bx][by]) {
                    bx = x;
                    by = y;
                }
            }
        }
        if (bx == -1) {
            break;
        } else {
            sum += ds[bx][by];
            dx[bx] = by;
            dy[by] = bx;
        }
    }
    return sum;
}

bool Solver3D_Demolition::DoDemolish() {
    while (freeBots > 0) {
        if (debug_3d_demolition) {
            std::cout << "Allocating new box\n";
        }
        if (!AllocateBox()) {
            break;
        }
    }
    if (targetBoxes.empty()) {
        return false;
    }
    CommandGroup g(state.active_bots.size(), {Command::Type::Wait});
    vector<vector<int>> ds;
    vector<DistMap*> distMaps;
    for (auto& box : targetBoxes) {
        for (auto& cd : box.cornerDistances) {
            distMaps.push_back(&cd);
            ds.emplace_back(state.active_bots.size(), 0);
            for (int i = 0; i < state.active_bots.size(); ++i) {
                ds.back()[i] = cd(GetBot(i).c);
            }
        }
    }
    vector<int> dx, dy;
    assignmentCost(ds, dx, dy);

    int xs = 0;
    for (auto boxIt = targetBoxes.begin(); boxIt != targetBoxes.end(); ++boxIt) {
        auto& box = *boxIt;
        bool done = true;
        for (auto& c : box.corners) {
            int x = xs++;
            done &= ds[x][dx[x]] == 0;
        }
        if (done) {
            for (int bi = 0; bi < state.active_bots.size(); ++bi) {
                if (box.cover(GetBot(bi).c)) {
                    // std::cout << "WTF, conflict\n";
                    done = false;
                }
            }
        }
        if (done) {
            if (debug_3d_demolition) {
                std::cout << "Box ready: " << box.corners[0] << " " << box.corners.back() << "\n";
            }
            xs -= box.corners.size();
            if (box.corners.size() == 1) {
                int bi = dx[xs];
                auto& bot = GetBot(bi);
                g[bi].type = Command::Type::GVoid;
                g[bi].cd1 = {box.corners[0].x - bot.c.x, box.corners[0].y - bot.c.y, box.corners[0].z - bot.c.z};
            } else {
                for (auto& c : box.corners) {
                    int x = xs++;
                    int bi = dx[x];
                    auto& bot = GetBot(bi);
                    g[bi].type = Command::Type::GVoid;
                    g[bi].cd1 = {c.x - bot.c.x, c.y - bot.c.y, c.z - bot.c.z};
                    g[bi].cd2 = {
                        (c.x == box.corners[0].x ? 1 : -1) * (box.corners.back().x - box.corners[0].x),
                        (c.y == box.corners[0].y ? 1 : -1) * (box.corners.back().y - box.corners[0].y),
                        (c.z == box.corners[0].z ? 1 : -1) * (box.corners.back().z - box.corners[0].z),
                    };
                }
            }
            targetBoxes.erase(boxIt);
            ExecuteCommands(g);
            return true;
        }
    }

    static const vector<int> dxs = {-1, 1, 0, 0, 0, 0};
    static const vector<int> dys = {0, 0, -1, 1, 0, 0};
    static const vector<int> dzs = {0, 0, 0, 0, -1, 1};
    unordered_set<Coordinate> conflicts;
    for (int bi = 0; bi < state.active_bots.size(); ++bi) {
        conflicts.insert(GetBot(bi).c);
    }
    for (int bi = 0; bi < state.active_bots.size(); ++bi) {
        auto& b = GetBot(bi);
        auto& distMap = dy[bi] == -1 ? distToBoundary : *distMaps[dy[bi]];
        for (int d = 0; d < 6; ++d) {
            Coordinate v = b.c;
            v.x += dxs[d];
            v.y += dys[d];
            v.z += dzs[d];
            if (matrix.IsInside(v) && matrix.Get(v) && !conflicts.count(v) &&
                distMap(v) + 1 < distMap(b.c)) {
                conflicts.insert(v);
                g[bi].type = Command::Type::Void;
                g[bi].cd1 = {dxs[d], dys[d], dzs[d]};
                goto cont;
            }
        }
        for (int d = 0; d < 6; ++d) {
            Coordinate v = b.c;
            vector<Coordinate> moveConficts;
            for (int len1 = 1; len1 <= TaskConsts::LONG_LIN_DIFF; ++len1) {
                v.x += dxs[d];
                v.y += dys[d];
                v.z += dzs[d];
                if (!matrix.IsInside(v) || matrix.Get(v) || conflicts.count(v)) {
                    break;
                }
                moveConficts.push_back(v);
                if (distMap(v) < distMap(b.c)) {
                    conflicts.insert(moveConficts.begin(), moveConficts.end());
                    g[bi].type = Command::Type::SMove;
                    g[bi].cd1 = {v.x - b.c.x, v.y - b.c.y, v.z - b.c.z};
                    goto cont;
                }
                if (len1 <= TaskConsts::SHORT_LIN_DIFF) {
                    for (int d1 = 0; d1 < 6; ++d1) {
                        if (d == d1) {
                            continue;
                        }
                        auto w = v;
                        vector<Coordinate> moveConficts2 = moveConficts;
                        for (int len2 = 1; len2 <= TaskConsts::SHORT_LIN_DIFF; ++len2) {
                            w.x += dxs[d1];
                            w.y += dys[d1];
                            w.z += dzs[d1];
                            if (!matrix.IsInside(w) || matrix.Get(w) || conflicts.count(w)) {
                                break;
                            }
                            moveConficts2.push_back(w);
                            if (distMap(w) < distMap(b.c)) {
                                conflicts.insert(moveConficts2.begin(), moveConficts2.end());
                                g[bi].type = Command::Type::LMove;
                                g[bi].cd1 = {v.x - b.c.x, v.y - b.c.y, v.z - b.c.z};
                                g[bi].cd2 = {w.x - v.x, w.y - v.y, w.z - v.z};
                                goto cont;
                            }
                        }
                    }
                }
            }
        }
        cont:;
    }
    bool noop = true;
    for (auto& c : g) {
        noop &= c.type == Command::Type::Wait;
    }
    if (noop) {
        std::cout << "Failed to do 3d demolition due to conflicts :(\n";
        if (debug_3d_demolition) {
            for (int x = 0; x < ds.size(); ++x) {
                for (int y = 0; y < ds[0].size(); ++y) {
                    std::cout << ds[x][y] << " ";
                }
                std::cout << "\n";
            }
        }
        throw StopException();
    }
    ExecuteCommands(g);
    return true;
}

bool Solver3D_Demolition::AllocateBox() {
    DistMap counts(matrix.GetR() + 1);
    for (int x = 0; x <= matrix.GetR(); ++x) {
        for (int y = 0; y <= matrix.GetR(); ++y) {
            for (int z = 0; z <= matrix.GetR(); ++z) {
                counts(x, y, z) = 0;
            }
        }
    }
    auto notCovered = [this](int x, int y, int z) -> bool {
        for (auto& box : targetBoxes) {
            if (box.cover(x, y, z)) {
                return false;
            }
        }
        return true;
    };
    for (int x = 0; x < matrix.GetR(); ++x) {
        for (int y = 0; y < matrix.GetR(); ++y) {
            for (int z = 0; z < matrix.GetR(); ++z) {
                counts(x + 1, y + 1, z + 1) =
                    + counts(x, y + 1, z + 1) + counts(x + 1, y, z + 1) + counts(x + 1, y + 1, z)
                    - counts(x + 1, y, z) - counts(x, y + 1, z) - counts(x, y, z + 1)
                    + counts(x, y, z);
                if (matrix.Get(x, y, z) && notCovered(x, y, z)) {
                    counts(x + 1, y + 1, z + 1)++;
                }
            }
        }
    }
    vector<DistMap> distMaps;
    for (int i = 0; i < state.active_bots.size(); ++i) {
        distMaps.emplace_back(SingleSourceShortestDists(matrix, GetBot(i).c, true));
    }
    vector<vector<int>> ds;
    for (auto& box : targetBoxes) {
        for (auto& c : box.corners) {
            ds.emplace_back(distMaps.size(), 0);
            for (int i = 0; i < distMaps.size(); ++i) {
                ds.back()[i] = distMaps[i](c);
            }
        }
    }
    vector<int> dx, dy;
    int assignment0 = assignmentCost(ds, dx, dy);
    TargetBox bestBox{-1, -1, -1, make_tuple(0, 0, 0)};
    double bestScore = numeric_limits<double>::infinity();
    // auto dim = make_tuple(30, 30, 30);
    int L = min(30, matrix.GetR() - 3);
    int L1 = min(30, matrix.GetR() - 2);
    const vector<tuple<int, int, int>> dims = {
        {L, L1, L},
        // {L, L1, 0},
        // {L, 0, L},
        // {L, L1, 0},
        // {L, 0, 0},
        // {0, L1, 0},
        // {0, 0, L},
        {0, 0, 0},
    };
    for (auto& dim : dims) {
        for (int x = 1; x + get<0>(dim) + 1 < matrix.GetR(); ++x) {
            for (int y = 0; y + get<1>(dim) + 1 < matrix.GetR(); ++y) {
                for (int z = 1; z + get<2>(dim) + 1 < matrix.GetR(); ++z) {
                    // for (auto& box : targetBoxes) {
                    //     if (box.)
                    // }
                    int cnt = counts(x + get<0>(dim) + 1, y + get<1>(dim) + 1, z + get<2>(dim) + 1)
                        - counts(x, y + get<1>(dim) + 1, z + get<2>(dim) + 1) - counts(x + get<0>(dim) + 1, y, z + get<2>(dim) + 1) - counts(x + get<0>(dim) + 1, y + get<1>(dim) + 1, z)
                        + counts(x + get<0>(dim) + 1, y, z) + counts(x, y + get<1>(dim) + 1, z) + counts(x, y, z + get<2>(dim) + 1)
                        - counts(x, y, z);
                    if (cnt == 0) {
                        continue;
                    }

                    TargetBox curBox(x, y, z, dim);
                    if (curBox.corners.size() > freeBots) {
                        break;
                    }
                    bool hasInvalidCorners = false;
                    for (auto& c : curBox.corners) {
                        ds.emplace_back(distMaps.size(), 0);
                        for (int i = 0; i < distMaps.size(); ++i) {
                            ds.back()[i] = distMaps[i](c);
                        }
                        bool validCorner = false;
                        for (int dx = -1; dx <= 1; ++dx) {
                            for (int dy = -1; dy <= 1; ++dy) {
                                for (int dz = -1; dz <= 1; ++dz) {
                                    if (dx * dx + dy * dy + dz * dz == 0 || dx * dx + dy * dy + dz * dz == 3 ||
                                        !matrix.IsInside(c.x + dx, c.y + dy, c.z + dz) ||
                                        bestBox.cover(c.x + dx, c.y + dy, c.z + dz) ||
                                        SomeBoxCovers(c.x + dx, c.y + dy, c.z + dz)) {
                                        continue;
                                    }
                                    validCorner = true;
                                }
                            }
                        }
                        hasInvalidCorners |= !validCorner;
                    }
                    if (!hasInvalidCorners) {
                        int assignment1 = assignmentCost(ds, dx, dy);
                        assert(assignment1 >= assignment0);
                        // TODO: better scoring?
                        double score = (30.0 * matrix.GetVolume() / TaskConsts::N_BOTS) * (assignment1 - assignment0);
                        score += 15.0 * ((get<0>(dim) + 1) * (get<1>(dim) + 1) * (get<2>(dim) + 1) - cnt);
                        score /= cnt;
                        if (score < 0) {
                            std::cout << (get<0>(dim) + 1) * (get<1>(dim) + 1) * (get<2>(dim) + 1) << " " << cnt << "\n";
                        }
                        if (score < bestScore) {
                            bestBox = curBox;
                            bestScore = score;
                        }
                    }
                    ds.erase(ds.end() - curBox.corners.size(), ds.end());
                }
            }
        }
    }
    if (isfinite(bestScore)) {
        if (debug_3d_demolition) {
            std::cout << "Allocated box: " << bestBox.corners[0] << " " << bestBox.corners.back() << " " << bestScore << "\n";
        }
        for (int i = 0; i < bestBox.corners.size(); ++i) {
            auto& c = bestBox.corners[i];
            vector<Coordinate> validPos;
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dz = -1; dz <= 1; ++dz) {
                        if (dx * dx + dy * dy + dz * dz == 0 || dx * dx + dy * dy + dz * dz == 3 ||
                            !matrix.IsInside(c.x + dx, c.y + dy, c.z + dz) ||
                            bestBox.cover(c.x + dx, c.y + dy, c.z + dz) ||
                            SomeBoxCovers(c.x + dx, c.y + dy, c.z + dz)) {
                            continue;
                        }
                        validPos.emplace_back(Coordinate{c.x + dx, c.y + dy, c.z + dz});
                    }
                }
            }
            bestBox.cornerDistances.emplace_back(SingleSourceShortestDists(matrix, validPos, true));
        }
        targetBoxes.push_back(bestBox);
        freeBots -= bestBox.corners.size();
        return true;
    }
    if (debug_3d_demolition) {
        std::cout << "Good box not found\n";
    }
    return false;
}

void Solver3D_Demolition::Flip() {
    CommandGroup g(state.active_bots.size(), {Command::Type::Wait});
    g[0].type = Command::Type::Flip;
    ExecuteCommands(g);
}

void Solver3D_Demolition::AllFission() {
    const int rows = 6;
    for (int i = 0; i < rows - 1; ++i) {
        CommandGroup g(state.active_bots.size(), {Command::Type::Wait});
        Command cmd = {Command::Type::Fission};
        cmd.cd1 = {1, 0, 0};
        cmd.m = GetBot(state.active_bots.size() - 1).seeds.size() - (TaskConsts::N_BOTS + i) / rows;
        g[state.active_bots.size() - 1] = cmd;
        ExecuteCommands(g);
    }
    while (true) {
        bool cont = false;
        CommandGroup g(state.active_bots.size(), {Command::Type::Wait});
        for (int i = 0; i < state.active_bots.size(); ++i) {
            auto& bot = GetBot(i);
            if (!bot.seeds.empty()) {
                cont = true;
                Command cmd = {Command::Type::Fission};
                cmd.cd1 = {0, 1, 0};
                cmd.m = bot.seeds.size() - 1;
                g[i] = cmd;
            }
        }
        if (cont) {
            ExecuteCommands(g);
        } else {
            break;
        }
    }
}

void Solver3D_Demolition::AllFusion() {
    static const vector<int> dxs = {-1, 1, 0, 0, 0, 0};
    static const vector<int> dys = {0, 0, -1, 1, 0, 0};
    static const vector<int> dzs = {0, 0, 0, 0, -1, 1};
    auto dist0 = SingleSourceShortestDists(matrix, {0, 0, 0}, false);
    while (state.active_bots.size() > 1 || GetBot(0).c != Coordinate{0, 0, 0}) {
        CommandGroup g(state.active_bots.size(), {Command::Type::Wait});
        unordered_set<Coordinate> conflicts;
        int zeroBot = -1;
        for (int bi = 0; bi < state.active_bots.size(); ++bi) {
            if (GetBot(bi).c == Coordinate{0, 0, 0}) {
                zeroBot = bi;
            }
            conflicts.insert(GetBot(bi).c);
        }
        auto ml = [](Coordinate c) {
            return abs(c.x) + abs(c.y) + abs(c.z);
        };
        for (int bi = 0; bi < state.active_bots.size(); ++bi) {
            auto& b = GetBot(bi);
            if (zeroBot != -1 && CoordinateDifference{b.c.x, b.c.y, b.c.z}.IsNearCoordinateDifferences()) {
                g[zeroBot].type = Command::Type::FusionP;
                g[zeroBot].cd1 = {b.c.x, b.c.y, b.c.z};
                g[bi].type = Command::Type::FusionS;
                g[bi].cd1 = {-b.c.x, -b.c.y, -b.c.z};
                zeroBot = -1;
                continue;
            }
            int16_t curDist = dist0(b.c);
            int curDist2 = ml(b.c);
            vector<Coordinate> curConflicts;
            for (int d = 0; d < 6; ++d) {
                Coordinate v = b.c;
                vector<Coordinate> moveConficts;
                for (int len1 = 1; len1 <= TaskConsts::LONG_LIN_DIFF; ++len1) {
                    v.x += dxs[d];
                    v.y += dys[d];
                    v.z += dzs[d];
                    if (!matrix.IsInside(v) || matrix.Get(v) || conflicts.count(v)) {
                        break;
                    }
                    moveConficts.push_back(v);
                    if (dist0(v) < curDist ||
                        (dist0(v) == curDist && ml(v) < curDist2)) {
                        curConflicts = moveConficts;
                        g[bi].type = Command::Type::SMove;
                        g[bi].cd1 = {v.x - b.c.x, v.y - b.c.y, v.z - b.c.z};
                        curDist = dist0(v);
                        curDist2 = ml(v);
                    }
                    if (len1 <= TaskConsts::SHORT_LIN_DIFF) {
                        for (int d1 = 0; d1 < 6; ++d1) {
                            if (d == d1) {
                                continue;
                            }
                            auto w = v;
                            vector<Coordinate> moveConficts2 = moveConficts;
                            for (int len2 = 1; len2 <= TaskConsts::SHORT_LIN_DIFF; ++len2) {
                                w.x += dxs[d1];
                                w.y += dys[d1];
                                w.z += dzs[d1];
                                if (!matrix.IsInside(w) || matrix.Get(w) || conflicts.count(w)) {
                                    break;
                                }
                                moveConficts2.push_back(w);
                                if (dist0(w) < curDist ||
                                    (dist0(w) == curDist && ml(w) < curDist2)) {
                                    curConflicts = moveConficts2;
                                    g[bi].type = Command::Type::LMove;
                                    g[bi].cd1 = {v.x - b.c.x, v.y - b.c.y, v.z - b.c.z};
                                    g[bi].cd2 = {w.x - v.x, w.y - v.y, w.z - v.z};
                                    curDist = dist0(w);
                                    curDist2 = ml(w);
                                }
                            }
                        }
                    }
                }
            }
            conflicts.insert(curConflicts.begin(), curConflicts.end());
        }
        ExecuteCommands(g);
    }
}

Evaluation::Result Solver3D_Demolition::Solve(const Matrix& target, Trace& output) {
    Solver3D_Demolition solver(target);
    solver.Solve();
    auto ret = Evaluation::Result(solver.state.IsCorrectFinal(), solver.state.energy);
    output.commands = solver.state.trace.commands;
    return ret;
}

void Solver3D_Demolition::ExecuteCommands(const CommandGroup& group) {
  int index = 0;
  for (const auto& c : group) {
    if (debug) {
      cout << index << ": adding command " << c << endl;
    }
    state.trace.commands.push_back(c);
    index += 1;
  }
  if (debug) {
    cout << "state.Step()" << endl;
  }
  state.Step();
}
