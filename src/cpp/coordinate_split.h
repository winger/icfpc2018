#pragma once

#include "base.h"
#include "matrix.h"

class CoordinateSplit
{
public:
    static vector<int> SplitUniform(int r, int n);
    static vector<int> SplitByVolume(const Matrix& matrix, int axis, int n);
};
