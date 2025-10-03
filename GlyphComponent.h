//
// Created by yoh on 28/09/25.
//

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

  explicit GlyphComponent(uint16_t numOfVertices,
                          std::set<uint16_t> endPtsOfContours,
                          BoundingRect boundingRect,
                          std::vector<glm::vec2> coordinates);
  [[nodiscard]] uint16_t getNumOfVertices() const;
  [[nodiscard]] std::set<uint16_t> getEndPtsOfContours() const;
  [[nodiscard]] BoundingRect getBoundingRect() const;
  [[nodiscard]] std::vector<glm::vec2> getCoordinates() const;
  void printDebugInfo() const;

private:
  uint16_t numOfVertices;
  std::set<uint16_t> endPtsOfContours;
  BoundingRect boundingRect;
  std::vector<glm::vec2> coordinates;
};

#endif  // GLYPHCOMPONENT_H
