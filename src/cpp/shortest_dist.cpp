#include "shortest_dist.h"
#include "constants.h"

DistMap SingleSourceShortestDists(Matrix const& matrix, int x0, int y0, int z0, bool dig) {
    static const vector<int> dxs = {-1, 1, 0, 0, 0, 0};
    static const vector<int> dys = {0, 0, -1, 1, 0, 0};
    static const vector<int> dzs = {0, 0, 0, 0, -1, 1};
    DistMap dist(matrix.GetR());
    dist(x0, y0, z0) = 0;
    vector<Coordinate> queues[4];
    queues[0].push_back({x0, y0, z0});
    int maxDist = 0;
    auto relax = [&](Coordinate c, int nd) {
        if (dist(c) == -1 || dist(c) > nd) {
            dist(c) = nd;
            queues[nd & 3].push_back(c);
        }
    };
    for (int curDist = 0; curDist <= maxDist; ++curDist) {
        auto& q = queues[curDist & 3];
        for (auto u : q) {
            if (dist(u) < curDist) {
                continue;
            }
            for (int d = 0; d < 6; ++d) {
                Coordinate v{u.x + dxs[d], u.y + dys[d], u.z + dzs[d]};
                if (!matrix.IsInside(v) || (!dig && matrix.Get(v))) {
                    continue;
                }
                int delta = matrix.Get(v) ? 2 : 1;
                relax(v, curDist + delta);
                for (int len1 = 2; len1 <= TaskConsts::LONG_LIN_DIFF; ++len1) {
                    v.x += dxs[d];
                    v.y += dys[d];
                    v.z += dzs[d];
                    if (!matrix.IsInside(v) || matrix.Get(v) || dist(v) <= curDist + delta - 1) {
                        break;
                    }
                    relax(v, curDist + delta);
                    if (len1 <= TaskConsts::SHORT_LIN_DIFF) {
                        auto w = v;
                        for (int d1 = 0; d1 < 6; ++d1) {
                            for (int len2 = 1; len2 <= TaskConsts::SHORT_LIN_DIFF; ++len2) {
                                w.x += dxs[d1];
                                w.y += dys[d1];
                                w.z += dzs[d1];
                                if (!matrix.IsInside(w) || matrix.Get(w) || dist(w) <= curDist + delta - 1) {
                                    break;
                                }
                                relax(w, curDist + delta);
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
