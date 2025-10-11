#include <fstream>

#include "FontParser.h"
#include "FrameBufferCanvas.h"
#include "utils/Unicode.h"

int main() {
  // Settings
  FontParser reader("fonts/NotoSansJP-Regular.ttf");
  const auto cps = utf8ToCodepoints("ほげ");
  constexpr int height = 300;

  // Get font metric data
  const auto [ascent, descent] = reader.getFontMetric();
  const float scale = static_cast<float>(height) / (ascent - descent);

  // Get glyphs and the necessary total width
  const auto [glyphs, width] = reader.getGlyphs(cps, scale);

  // Initialize canvas and render glyphs
  FrameBufferCanvas canvas{width, height};
  canvas.setGlyphBaseline(ascent);
  canvas.setScale(scale);
  canvas.renderGlyphs(glyphs);
  canvas.writePngFile("out.png");
}
