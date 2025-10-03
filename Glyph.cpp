//
// Created by yoh on 03/10/25.
//

#include "Glyph.h"

#include <utility>

Glyph::Glyph(std::vector<GlyphComponent> components_) :
  components(std::move(components_)) {
  assert(!components.empty());
  rect = components[0].getBoundingRect();
}

Glyph::Glyph(std::vector<GlyphComponent> components_,
             const BoundingRect rect_) :
  components(std::move(components_)),
  rect(rect_) {
}

void Glyph::printDebugInfo() const {
  for (const auto& c : components) c.printDebugInfo();
}

std::vector<GlyphComponent> Glyph::getComponents() const {
  return components;
}