//
// Created by yoh on 01/10/25.
//

#ifndef GEOMETORY_H
#define GEOMETORY_H

#include <iostream>

struct BoundingRect {
  int xMin = 0;
  int xMax = 0;
  int yMin = 0;
  int yMax = 0;
};

inline std::ostream& operator<<(std::ostream& out, const BoundingRect& b) {
  out << b.xMin << ":" << b.xMax << ":" << b.yMin << ":" << b.yMax <<
      std::endl;
  return out;
}

#endif //GEOMETORY_H
