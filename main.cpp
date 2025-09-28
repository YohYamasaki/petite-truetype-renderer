#include <fstream>

#include "FontReader.h"
#include "FrameBufferCanvas.h"


int main() {
  FontReader reader("fonts/JetBrainsMono-Bold.ttf");
  FrameBufferCanvas canvas{1200, 1200};
  const Glyph glyph = reader.getGlyph('j');
  glyph.printDebugInfo();
  canvas.renderGlyph(300, 200, glyph, RGB{255});

  canvas.writePngFile("hoge.png");
  return 0;
}
