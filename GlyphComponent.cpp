//
// Created by yoh on 28/09/25.
//

#include "GlyphComponent.h"

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

GlyphComponent::GlyphComponent(uint16_t numOfVertices,
                               std::set<uint16_t> endPtsOfContours,
                               BoundingRect boundingRect,
                               std::vector<glm::vec2> coordinates)
  : numOfVertices(numOfVertices),
    endPtsOfContours(std::move(endPtsOfContours)),
    boundingRect(boundingRect),
    coordinates(std::move(coordinates)) {
}

uint16_t GlyphComponent::getNumOfVertices() const {
  return numOfVertices;
}

std::set<uint16_t> GlyphComponent::getEndPtsOfContours() const {
  return endPtsOfContours;
}


BoundingRect GlyphComponent::getBoundingRect() const {
  return boundingRect;
}

std::vector<glm::vec2> GlyphComponent::getCoordinates() const {
  return coordinates;
}

void GlyphComponent::printDebugInfo() const {
  std::wcout << "numOfVertices: " << numOfVertices << std::endl;

  std::wcout << "endPtsOfContours: [";
  for (auto c : endPtsOfContours) {
    std::wcout << c << ",";
  }
  std::wcout << "]" << std::endl;

  for (int i = 0; i < numOfVertices; ++i) {
    std::wcout << "points " << i << ": ";
    std::wcout << coordinates[i][0] << ", " << coordinates[i][1] << std::endl;
  }
}

