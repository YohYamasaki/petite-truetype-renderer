//
// Created by yoh on 28/09/25.
//

#pragma once
#ifndef GLYPHCOMPONENT_H
#define GLYPHCOMPONENT_H
#include <cstdint>
#include <set>
#include <vector>
#include <glm/vec2.hpp>

#include "utils/Geometry.h"


class GlyphComponent {
public:
  GlyphComponent() : numOfVertices(0) {
  }

  explicit GlyphComponent(uint16_t numOfVertices_,
                          std::set<uint16_t> endPtsOfContours_,
                          std::set<uint16_t> ptsOnCurve_,
                          BoundingRect boundingRect_,
                          std::vector<glm::vec2> coordinates_);
  [[nodiscard]] uint16_t getNumOfVertices() const;
  [[nodiscard]] std::set<uint16_t> getEndPtsOfContours() const;
  [[nodiscard]] std::set<uint16_t> getPtsOnCurve() const;
  [[nodiscard]] BoundingRect getBoundingRect() const;
  [[nodiscard]] std::vector<glm::vec2> getCoordinates() const;
  void printDebugInfo() const;

private:
  uint16_t numOfVertices;
  std::set<uint16_t> endPtsOfContours;
  BoundingRect boundingRect;
  std::vector<glm::vec2> coordinates;
  std::set<uint16_t> ptsOnCurve;
};

#endif  // GLYPHCOMPONENT_H
