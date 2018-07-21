#pragma once

#include "matrix.h"

class Grounder
{
public:
    static bool IsGrounded(Matrix const& model);
    static bool Check(unsigned model_index);
    static void CheckAll();
};
