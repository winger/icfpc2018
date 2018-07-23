#pragma once

#include "matrix.h"
#include "trace.h"

class AutoHarmonic
{
public:
    void ImproveTrace(const Matrix& source, const Matrix& target, const Trace& trace, Trace& output);
};
