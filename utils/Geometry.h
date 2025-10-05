//
// Created by yoh on 01/10/25.
//

#pragma once
#ifndef GEOMETORY_H
#define GEOMETORY_H

#include <cmath>
#include <iostream>
#include <optional>

#include "Debug.h"
#include "glm/glm.hpp"

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
quadBezierLerp(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c,
               const float t) {
  assert(t >= 0 && t <= 1);
  return lerp(lerp(a, b, t), lerp(b, c, t), t);
}

inline std::optional<glm::vec2> segmentsIntersect(
    const glm::vec2& a1, const glm::vec2& a2,
    const glm::vec2& b1, const glm::vec2& b2) {
  //  >0: c is above of start-end line, <0: below, ==0: collinear
  const auto region = [](const glm::vec2& pt, const glm::vec2& start,
                         const glm::vec2& end) {
    return (end.y - start.y) * (pt.x - start.x) - (end.x - start.x) * (
             pt.y - start.y);
  };
  const auto ra = region(b1, a1, a2) * region(b2, a1, a2) < 0;
  const auto rb = region(a1, b1, b2) * region(a2, b1, b2) < 0;

  if (ra && rb) {
    const glm::vec2 v1 = a2 - a1;
    const glm::vec2 v2 = b2 - b1;

    // 2D scalar cross product
    auto cross = [](const glm::vec2& u, const glm::vec2& v) {
      return u.x * v.y - u.y * v.x;
    };

    const auto denom = cross(v1, v2);
    constexpr auto eps = 1e-8f;
    if (std::fabs(denom) < eps) return std::nullopt; // parallel / numeric guard

    const auto t = cross(b1 - a1, v2) / denom;
    return a1 + t * v1;
  }
  return std::nullopt;
}

inline std::vector<float> solveQuadratic(const float a, const float b,
                                         const float c) {
  constexpr float eps = 1e-8f;
  std::vector<float> roots;
  if (std::fabs(a) < eps) {
    // linear
    if (std::fabs(b) < eps) return roots;
    roots.push_back(-c / b);
    return roots;
  }
  const float D = b * b - 4.0f * a * c;
  if (D < -eps) return roots; // no real roots
  if (D < 0.0f) {
    // near zero
    roots.push_back(-b / (2.0f * a));
    return roots;
  }
  const float sqrtD = std::sqrt(D);
  const float q = -0.5f * (b + (b >= 0.0f ? sqrtD : -sqrtD));
  const float r1 = q / a;
  const float r2 = c / q;
  roots.push_back(r1);
  if (std::fabs(r2 - r1) > 1e-7f) roots.push_back(r2);
  return roots;
}

inline std::vector<glm::vec2> segmentQuadBezierIntersect(
    const glm::vec2& p1, // bezier start
    const glm::vec2& p2, // bezier control
    const glm::vec2& p3, // bezier end
    const glm::vec2& l1, // line start
    const glm::vec2& l2 // line end
    ) {
  const auto k = glm::vec2((l2 - l1).y, -(l2 - l1).x);
  const auto a = p3 - 2.0f * p2 + p1;
  const auto b = 2.0f * (p2 - p1);
  const auto c = p1;

  const auto ts = solveQuadratic(
      dot(k, a),
      dot(k, b),
      dot(k, c - l1)
      );

  std::vector<glm::vec2> intersects(0);
  for (const auto t : ts) {
    if (t >= 0 && t < 1) intersects.push_back(a * t * t + b * t + c);
  }
  return intersects;
}

#endif //GEOMETORY_H
