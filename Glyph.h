//
// Created by yoh on 28/09/25.
//

#ifndef GLYPH_H
#define GLYPH_H
#include <cstdint>
#include <set>
#include <vector>

#include "utils/Geometry.h"


class Glyph {
public:
  explicit Glyph(uint16_t numOfVertices,
                 std::set<uint16_t> endPtsOfContours,
                 BoundingRect boundingRect,
                 std::vector<int> xCoordinates,
                 std::vector<int> yCoordinates);
  [[nodiscard]] uint16_t getNumOfVertices() const;
  [[nodiscard]] std::vector<int> getXCoordinates() const;
  [[nodiscard]] std::vector<int> getYCoordinates() const;
  [[nodiscard]] std::set<uint16_t> getEndPtsOfContours() const;
  [[nodiscard]] BoundingRect getBoundingRect() const;

  void printDebugInfo() const;
  static Glyph EmptyGlyph();

private:
  uint16_t numOfVertices;
  std::set<uint16_t> endPtsOfContours;
  BoundingRect boundingRect;
  std::vector<int> xCoordinates;
  std::vector<int> yCoordinates;
};

#endif  // GLYPH_H
