//
// Created by yoh on 03/10/25.
//

#pragma once
#ifndef GLYPH_H
#define GLYPH_H
#include "GlyphComponent.h"


class Glyph {
public:
  Glyph() = default;
  explicit Glyph(std::vector<GlyphComponent> components_);
  explicit Glyph(std::vector<GlyphComponent> components_, BoundingRect rect);
  [[nodiscard]] std::vector<GlyphComponent> getComponents() const;
  void printDebugInfo() const;

private:
  std::vector<GlyphComponent> components;
  BoundingRect rect;
};


#endif //GLYPH_H
