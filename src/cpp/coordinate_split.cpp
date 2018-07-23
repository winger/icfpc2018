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
        size_t max = 0;
        size_t s2 = 0;
        int from = 0;
    };

    assert((axis == 1) || (axis == 2) || (axis == 3));
    int r = matrix.GetR(), r1 = r + 1;
    vector<size_t> volume(r1, 0);
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
        svolume[i] = svolume[i-1] + volume[i];
    svolume.push_back(svolume.back());
    vector<int> vi;
    vi.push_back(0);
    for (int i = 1; i < r; ++i)
    {
        if (volume[i] > 0)
            vi.push_back(i);
    }
    vi.push_back(r);

    vector<vector<SplitInfo>>vdp(n, vector<SplitInfo>(vi.size()));
    for (unsigned k = 0; k < vi.size(); ++k)
    {
        int i = vi[i];
        size_t v = svolume[i + 1];
        // vdp[0][k]
    }
}
