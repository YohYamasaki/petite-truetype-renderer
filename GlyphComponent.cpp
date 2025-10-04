//
// Created by yoh on 28/09/25.
//

#include "GlyphComponent.h"

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

GlyphComponent::GlyphComponent(uint16_t numOfVertices_,
                               std::set<uint16_t> endPtsOfContours_,
                               std::set<uint16_t> ptsOnCurve_,
                               BoundingRect boundingRect_,
                               std::vector<glm::vec2> coordinates_)
  : numOfVertices(numOfVertices_),
    endPtsOfContours(std::move(endPtsOfContours_)),
    boundingRect(boundingRect_),
    coordinates(std::move(coordinates_)),
    ptsOnCurve(std::move(ptsOnCurve_)) {
}

uint16_t GlyphComponent::getNumOfVertices() const {
  return numOfVertices;
}

std::set<uint16_t> GlyphComponent::getEndPtsOfContours() const {
  return endPtsOfContours;
}

std::set<uint16_t> GlyphComponent::getPtsOnCurve() const {
  return ptsOnCurve;
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

