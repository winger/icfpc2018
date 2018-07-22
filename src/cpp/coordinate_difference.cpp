#include "coordinate_difference.h"

ostream& operator<<(ostream& s, const CoordinateDifference& cd) {
    s << "(" << cd.dx << ", " << cd.dy << ", " << cd.dz << ")";
    return s;
}
