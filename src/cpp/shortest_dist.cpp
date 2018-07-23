#include "shortest_dist.h"
#include "constants.h"


DistMap SingleSourceShortestDists(Matrix const& matrix, Coordinate source, bool dig) {
    return SingleSourceShortestDists(matrix, vector<Coordinate>{source}, dig);
}

DistMap SingleSourceShortestDists(Matrix const& matrix, vector<Coordinate> sources, bool dig) {
    static const vector<int> dxs = {-1, 1, 0, 0, 0, 0};
    static const vector<int> dys = {0, 0, -1, 1, 0, 0};
    static const vector<int> dzs = {0, 0, 0, 0, -1, 1};
    DistMap dist(matrix.GetR());
    vector<Coordinate> queues[4];
    for (auto& source : sources) {
        dist(source) = 0;
        queues[0].push_back(source);
    }
    int maxDist = 0;
    auto relax = [&](Coordinate c, int nd) {
        if (dist(c) == -1 || dist(c) > nd) {
            dist(c) = nd;
            queues[nd & 3].push_back(c);
            maxDist = max(maxDist, nd);
        }
    };
    for (int curDist = 0; curDist <= maxDist; ++curDist) {
        auto& q = queues[curDist & 3];
        for (auto u : q) {
            if (dist(u) < curDist) {
                continue;
            }
            if (dig && matrix.Get(u)) {
                for (int d = 0; d < 6; ++d) {
                    Coordinate v{u.x + dxs[d], u.y + dys[d], u.z + dzs[d]};
                    if (!matrix.IsInside(v)) {
                        continue;
                    }
                    relax(v, curDist + 2);
                }
                continue;
            }
            for (int d = 0; d < 6; ++d) {
                Coordinate v = u;
                for (int len1 = 1; len1 <= TaskConsts::LONG_LIN_DIFF; ++len1) {
                    v.x += dxs[d];
                    v.y += dys[d];
                    v.z += dzs[d];
                    if (!matrix.IsInside(v) || (!dig && matrix.Get(v)) ||
                        (dist(v) != -1 && dist(v) <= curDist)) {
                        break;
                    }
                    relax(v, curDist + 1);
                    if (matrix.Get(v)) {
                        break;
                    }
                    if (len1 <= TaskConsts::SHORT_LIN_DIFF) {
                        for (int d1 = 0; d1 < 6; ++d1) {
                            auto w = v;
                            for (int len2 = 1; len2 <= TaskConsts::SHORT_LIN_DIFF; ++len2) {
                                w.x += dxs[d1];
                                w.y += dys[d1];
                                w.z += dzs[d1];
                                if (!matrix.IsInside(w) || (!dig && matrix.Get(w)) ||
                                    (dist(v) != -1 && dist(w) <= curDist)) {
                                    break;
                                }
                                relax(w, curDist + 1);
                                if (matrix.Get(w)) {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        q.clear();
    }
    return dist;
}
