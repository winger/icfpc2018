#pragma once

#include "matrix.h"
#include "trace.h"

class AutoHarmonic
{
public:
    static void ImproveTrace(const Matrix& source, const Matrix& target, const Trace& trace, Trace& output);
};
