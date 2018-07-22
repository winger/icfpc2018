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
