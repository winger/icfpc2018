#include "layers_lines.h"

void AssemblySolverLayersLines::SetRegion(const Region& _region, const Coordinate& target)
{
    region = _region;
    assert(region.a.x <= target.x && target.x << region.b.x);
    assert(region.a.y <= target.y && target.y <<  region.b.y);
    assert(region.a.z <= target.z && target.z <<  region.b.z);
    AssemblySolverLayersBase::targetC = target;
}

