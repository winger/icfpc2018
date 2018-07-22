#include "coordinate.h"

ostream& operator<<(ostream& s, const Coordinate& c) {
  s << "{x = " << c.x << ", y = " << c.y << ", z = " << c.z << "}";
  return s;
}
