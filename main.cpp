#include <fstream>

#include "FontReader.h"
#include "FrameBufferCanvas.h"

int main() {
  FontReader reader("fonts/JetBrainsMono-Bold.ttf");
  FrameBufferCanvas canvas{1200, 1200};
  const Glyph glyph = reader.getGlyph("ệ");
  canvas.renderGlyph(300, 300, glyph, RGB{255});

  canvas.writePngFile("out.png");
  return 0;
}
