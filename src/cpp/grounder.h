#pragma once

#include "matrix.h"

class Grounder
{
public:
    static bool IsDeltaGrounded(
      Matrix const& model,
      std::vector<uint32_t> const& indicies);
    static bool IsLayerGrounded(Matrix const& model);
    static bool Check(unsigned model_index);
    static void CheckAll();
};
