#include "coordinate_split.h"

vector<int> CoordinateSplit::SplitUniform(int r, int n)
{
    vector<int> split;
    if (r <= n)
    {
        for (int i = 0; i <= r; ++i)
            split.push_back(i);
    }
    else
    {
        for (int i = 0; i <= n; ++i)
            split.push_back((i*r) / n);
    }
    return split;
}

vector<int> CoordinateSplit::SplitByVolume(const Matrix& matrix, int axis, int n)
{
    struct SplitInfo 
    {
        size_t max = size_t(-1);
        size_t s2 = size_t(-1);
        unsigned from = 0;

        bool operator<(const SplitInfo& si) const { return (max < si.max) || ((max == si.max) && (s2 < si.s2)); }
    };

    assert((axis == 1) || (axis == 2) || (axis == 3));
    int r = matrix.GetR(), r1 = r + 1;
    vector<size_t> volume(r, 0);
    for (int x = 0; x < r; ++x)
    {
        for (int y = 0; y < r; ++y)
        {
            for (int z = 0; z < r; ++z)
            {
                if (matrix.Get(x, y, z))
                {
                    volume[(axis == 1) ? x : (axis == 2) ? y : z] += 1;
                }
            }
        }
    }
    vector<size_t> svolume(r1, 0);
    for (int i = 1; i < r1; ++i)
        svolume[i] = svolume[i-1] + volume[i-1];
    vector<int> vi;
    for (int i = 1; i < r; ++i)
    {
        if (volume[i] > 0)
            vi.push_back(i);
    }
    vi.push_back(r);
    if (vi.size() <= n)
    {  
        vi.insert(vi.begin(), 0);
        return vi;
    }

    vector<vector<SplitInfo>>vdp(n, vector<SplitInfo>(vi.size()));
    for (unsigned k = 0; k < vi.size(); ++k)
    {
        int i = vi[k];
        size_t v = svolume[i];        
        vdp[0][k].max = v;
        vdp[0][k].s2 = v*v;
    }
    SplitInfo best = vdp[0].back(); unsigned best_l = 0;
    for (unsigned l = 1; l < unsigned(n); ++l)
    {
        for (unsigned k = l; k < vi.size(); ++k)
        {
            int i = vi[k];
            for (unsigned m = 0; m < k; ++m)
            {
                int j = vi[m];
                size_t v = svolume[i] - svolume[j];
                const SplitInfo& sdp = vdp[l-1][m];
                SplitInfo si { max(v, sdp.max), v*v + sdp.s2, m};
                if (si < vdp[l][k])
                    vdp[l][k] = si;
            }
        }
        if (vdp[l].back() < best)
        {
            best = vdp[l].back();
            best_l = l;
        }
    }
    vector<int> output;
    {
        unsigned k = vi.size() - 1, l = best_l;
        for (; ; --l)
        {
            output.push_back(vi[k]);
            k = vdp[l][k].from;
            if (l == 0)
                break;
        }
        output.push_back(0);
    }
    reverse(output.begin(), output.end());
    // cout << "Output: " << output.size() << " " << output[0] << " " << output[1] << " ... " << output[output.size() - 2] << " " << output[output.size() - 1] << endl;
    return output;
}


// Old code from lasyers_parallel just for history.
// It find optimal split based on solutions, not volume. There is a chance that we will reuse it later for small tests.
// void AssemblySolverLayersParallel::FindBestSplit()
// {
//     struct SplitInfo
//     {
//         bool valid = false;
//         size_t moves;
//         uint64_t energy;
//         vector<int> splits;
//     };

//     int r = matrix.GetR();
//     split_axis = 1;
//     split_coordinate.resize(0);
//     for (unsigned i = 0; i <= TaskConsts::N_BOTS; ++i)
//         split_coordinate.push_back((i * r) / TaskConsts::N_BOTS);
//     if (!search_best_split) return;

//     Matrix mtemp; mtemp.Init(r);
//     bool best_split_valid = false;
//     size_t best_moves = 0, best_energy = 0;

//     for (int axis = 1; axis <= 3; axis += 2)
//     {
//         Timer t;
//         // don't check all positions because too expensive
//         vector<size_t> volume(r, 0);
//         size_t total_volume = 0;
//         for (int x = 0; x < r; ++x)
//         {
//             for (int y = 0; y < r; ++y)
//             {
//                 for (int z = 0; z < r; ++z)
//                 {
//                     if (matrix.Get(x, y, z))
//                     {
//                         volume[(axis == 1) ? x : z] += 1;
//                         total_volume += 1;
//                     }
//                 }
//             }
//         }
//         vector<size_t> volume2(r + 1, 0);
//         volume2[0] = volume2[r] = total_volume;
//         size_t last_volume = volume[0];
//         for (int i = 1; i < r; ++i)
//         {
//             if (volume[i] > 0)
//             {
//                 volume2[i] = volume[i] + last_volume;
//                 last_volume = volume[i];
//             }
//         }

//         vector<pair<size_t, int>> vp;
//         for (int i = 0; i <= r; ++i)
//         {
//             if (volume2[i] > 0)
//             {
//                 vp.push_back({volume2[i], i});
//             }
//         }
//         sort(vp.begin(), vp.end());
//         reverse(vp.begin(), vp.end());

//         Trace trace;
//         vector<vector<pair<size_t, uint64_t>>> vcost(r, vector<pair<size_t, uint64_t>>(r + 1, {0, 0}));
//         for (unsigned i0 = 1; i0 < vp.size(); ++i0)
//         {
//             for (unsigned j0 = 0; j0 < i0; ++j0)
//             {
//                 int i = min(vp[i0].second, vp[j0].second);
//                 int j = max(vp[i0].second, vp[j0].second);
//                 int x0 = (axis == 1) ? i : 0;
//                 int x1 = (axis == 1) ? j : r;
//                 int z0 = (axis == 3) ? i : 0;
//                 int z1 = (axis == 3) ? j : r;
//                 for (int x = x0; x < x1; ++x)
//                 {
//                     for (int y = 0; y < r; ++y)
//                     {
//                         for (int z = z0; z < z1; ++z)
//                         {
//                             if (matrix.Get(x, y, z))
//                                 mtemp.Fill(x, y, z);
//                         }
//                     }
//                 }
//                 uint64_t energy = AssemblySolverLayersBase::SolveHelper(mtemp, {(axis == 1) ? i : 0, 0, (axis == 3) ? i : 0}, trace).energy;
//                 size_t moves = trace.size();
//                 energy -= 30 * matrix.GetVolume() * moves; // We will pay for moves later
//                 energy -= 20 * moves; // We will pay for bot-moves later
//                 vcost[i][j] = {moves, energy};
//                 for (int x = x0; x < x1; ++x)
//                 {
//                     for (int y = 0; y < r; ++y)
//                     {
//                         for (int z = z0; z < z1; ++z)
//                         {
//                             mtemp.Erase(x, y, z);
//                         }
//                     }
//                 }

//                 if (t.GetMilliseconds() > max_time_for_search / 2)
//                     break;
//             }
//             if (t.GetMilliseconds() > max_time_for_search / 2)
//                 break;
//         }

//         vector<vector<SplitInfo>> vdp(TaskConsts::N_BOTS, vector<SplitInfo>(r+1));
//         for (int k = 1; k <= r; ++k)
//         {
//             if (vcost[0][k].first == 0) continue;
//             vdp[0][k].valid = true;
//             vdp[0][k].moves = vcost[0][k].first;
//             vdp[0][k].energy = vcost[0][k].second;
//             vdp[0][k].splits = {0, k};
//         }
//         for (unsigned l = 1; l < TaskConsts::N_BOTS; ++l)
//         {
//             for (int m = l; m < r; ++m)
//             {
//                 if (!vdp[l-1][m].valid) continue;
//                 for (int k = m + 1; k <= r; ++k)
//                 {
//                     size_t new_moves = vcost[m][k].first;
//                     if (new_moves == 0) continue;
//                     size_t extra_moves = 4 * l + 2 * (m / 15);
//                     size_t total_moves = max(vdp[l-1][m].moves, new_moves + extra_moves);
//                     size_t energy = vdp[l-1][m].energy + vcost[m][k].second;
//                     if (!vdp[l][k].valid || (vdp[l][k].moves > total_moves) || ((vdp[l][k].moves == total_moves) && (vdp[l][k].energy > energy)))
//                     {
//                         vdp[l][k].valid = true;
//                         vdp[l][k].moves = total_moves;
//                         vdp[l][k].energy = energy;
//                         vdp[l][k].splits = vdp[l-1][m].splits;
//                         vdp[l][k].splits.push_back(k);
//                     }
//                 }
//             }
//         }
//         for (unsigned l = 0; l < TaskConsts::N_BOTS; ++l)
//         {
//             if (!vdp[l][r].valid)
//                 continue;
//             size_t energy = vdp[l][r].energy + 20 * l * vdp[l][r].moves; // penaly for bots
//             if (!best_split_valid || (best_moves > vdp[l][r].moves) || ((best_moves == vdp[l][r].moves) && (best_energy > energy)))
//             {
//                 best_split_valid = true;
//                 best_moves = vdp[l][r].moves;
//                 best_energy = energy;
//                 split_axis = axis;
//                 split_coordinate = vdp[l][r].splits;
//             }
//         }
//     }
//     // cout << "Split: " << split_axis << " " << split_coordinate.size() << " " << best_moves << endl;
//     // assert(best_split_valid);
// }

