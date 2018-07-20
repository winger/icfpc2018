#pragma once

#include "matrix.h"
#include "trace.h"

class Evaluation
{
public:
    static uint64_t CheckSolution(const Matrix& model, const Trace& t);

    static void TestDfltSolution(unsigned model_index);
    static void TestAllDfltSolution();
};
