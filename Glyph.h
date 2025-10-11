#pragma once
#ifndef GLYPH_H
#define GLYPH_H
#include "GlyphComponent.h"

struct Metric {
  uint16_t advanceWidth;
  int16_t leftSideBearing;
};

class Glyph {
public:
  Glyph() = default;
  explicit Glyph(std::vector<GlyphComponent> components_, Metric metric_);
  [[nodiscard]] const std::vector<GlyphComponent>& getComponents() const;
  [[nodiscard]] const Metric& getMetric() const;
  static Glyph EmptyGlyph(Metric metric_);

private:
  std::vector<GlyphComponent> components;
  Metric metric;
};


#endif //GLYPH_H
