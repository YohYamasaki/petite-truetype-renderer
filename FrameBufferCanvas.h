//
// Created by Yoh Yamasaki on 30/09/2025.
//

#ifndef PETITE_TRUETYPE_RENDERER_FRAMEBUFFERCANVAS_H
#define PETITE_TRUETYPE_RENDERER_FRAMEBUFFERCANVAS_H
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <memory>

#include "Glyph.h"

struct RGB {
  unsigned char r, g, b;
  RGB() = default;

  constexpr RGB(const unsigned char r_, const unsigned char g_,
                const unsigned char b_) : r(r_), g(g_), b(b_) {
  }

  constexpr explicit RGB(const unsigned char greyscale) : r(greyscale),
    g(greyscale), b(greyscale) {
  }
};


class FrameBufferCanvas {
public:
  FrameBufferCanvas(unsigned int width, unsigned int height);
  [[nodiscard]] unsigned int width() const;
  [[nodiscard]] unsigned int height() const;
  void set(unsigned int x, unsigned int y, RGB color);
  void rect(unsigned int centerX, unsigned int centerY, unsigned int w,
            unsigned int h, RGB color);
  void line(int ax, int ay, int bx, int by,
            int thickness, RGB color);
  void renderGlyph(unsigned int startX,
                   unsigned int startY,
                   const Glyph& glyph,
                   RGB color);
  void writePngFile(const char* fileName) const;

private:
  unsigned int width_;
  unsigned int height_;
  std::unique_ptr<RGB[]> framebuffer;
};


#endif //PETITE_TRUETYPE_RENDERER_FRAMEBUFFERCANVAS_H