#include <fstream>

#include "FontReader.h"
#include "FrameBufferCanvas.h"

int main() {
  // FontReader reader("fonts/JetBrainsMono-Bold.ttf");
  // FrameBufferCanvas canvas{1200, 1200};
  // const Glyph glyph = reader.getGlyph("a");
  // canvas.renderGlyphOutline(glm::vec2(300, 300), glyph, RGB{255});

  // canvas.writePngFile("out.png");

  const auto intersection = segmentsIntersect(
      glm::vec2(-5, 5),
      glm::vec2(4, 0),
      glm::vec2(-4, -3),
      glm::vec2(0, 4)
      );
  if (intersection.has_value()) {
    const auto i = intersection.value();
    std::cout << "intersection: " << i.x << ":" << i.y << std::endl;
  }
  return 0;
}
