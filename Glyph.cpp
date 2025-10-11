#include "Glyph.h"

#include <utility>

Glyph::Glyph(std::vector<GlyphComponent> components_,
             const Metric metric_) :
  components(std::move(components_)),
  metric(metric_) {
}

const std::vector<GlyphComponent>& Glyph::getComponents() const {
  return components;
}

const Metric& Glyph::getMetric() const {
  return metric;
}

Glyph Glyph::EmptyGlyph(const Metric metric_) {
  return Glyph({}, metric_);
}