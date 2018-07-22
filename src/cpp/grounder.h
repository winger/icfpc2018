#pragma once

#include "matrix.h"

class Grounder
{
public:
    static bool IsDeltaGrounded(
      Matrix const& model,
      std::vector<int32_t> const& indicies);
    static bool IsByLayerGrounded(Matrix const& model);
    static bool IsProjectionGrounded(Matrix const& model);
    static bool IsGrounded(Matrix const& model);
    static bool Check(unsigned model_index);
    static void CheckAll();
};
