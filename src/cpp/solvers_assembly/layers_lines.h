#pragma once

#include "layers_base.h"
#include "../matrix.h"
#include "../region.h"

class AssemblySolverLayersLines : public AssemblySolverLayersBase
{
protected:
    Region region;

    void SetRegion(const Region& region, const Coordinate& target);

    void SolveInit();
    void SolveFinalize();

    void SolveLayer(int y);
};
