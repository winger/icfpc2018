#pragma once

#include "matrix.h"
#include "trace.h"

class Evaluation
{
public:
    struct Result
    {
        bool correct = false;
        uint64_t energy;  
    };

    static Result Evaluate(const Matrix& source, const Matrix& target, const Trace& t);
    
    static uint64_t CheckSolution(const Matrix& model, const Trace& t);

    static void TestDfltSolution(unsigned model_index);
    static void TestAllDfltSolution();
};
