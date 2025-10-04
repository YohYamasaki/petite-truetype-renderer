//
// Created by yoh on 01/10/25.
//

#pragma once
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

inline glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, const float t) {
  assert(t >= 0 && t <= 1);
  return a + (b - a) * t;
}

inline glm::vec2
bezierLerp(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c,
           const float t) {
  assert(t >= 0 && t <= 1);
  return lerp(lerp(a, b, t), lerp(b, c, t), t);
}

#endif //GEOMETORY_H
