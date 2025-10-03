//
// Created by yoh on 28/09/25.
//

#include "Glyph.h"

#include <iostream>
#include <utility>
#include <vector>

Glyph::Glyph(uint16_t numOfVertices,
             std::set<uint16_t> endPtsOfContours,
             BoundingRect boundingRect,
             std::vector<int> xCoordinates,
             std::vector<int> yCoordinates) {
  this->numOfVertices = numOfVertices;
  this->endPtsOfContours = std::move(endPtsOfContours);
  this->boundingRect = boundingRect;
  this->xCoordinates = std::move(xCoordinates);
  this->yCoordinates = std::move(yCoordinates);
}

uint16_t Glyph::getNumOfVertices() const {
  return numOfVertices;
}

std::vector<int> Glyph::getXCoordinates() const {
  return xCoordinates;
}

std::vector<int> Glyph::getYCoordinates() const {
  return yCoordinates;
}

std::set<uint16_t> Glyph::getEndPtsOfContours() const {
  return endPtsOfContours;
}


BoundingRect Glyph::getBoundingRect() const {
  return boundingRect;
}

void Glyph::printDebugInfo() const {
  std::wcout << "numOfVertices: " << numOfVertices << std::endl;

  std::wcout << "endPtsOfContours: [";
  for (auto c : endPtsOfContours) {
    std::wcout << c << ",";
  }
  std::wcout << "]" << std::endl;

  for (int i = 0; i < numOfVertices; ++i) {
    std::wcout << "points " << i << ": ";
    std::wcout << xCoordinates[i] << ", " << yCoordinates[i] << std::endl;
  }
}

Glyph Glyph::EmptyGlyph() {
  const std::set<uint16_t> emptySet;
  constexpr BoundingRect rect;
  constexpr std::vector<int> emptyVec;
  return Glyph{0, emptySet, rect, emptyVec, emptyVec};
}
