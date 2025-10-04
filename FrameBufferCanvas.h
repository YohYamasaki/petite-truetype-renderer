//
// Created by Yoh Yamasaki on 30/09/2025.
//

#pragma once
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

constexpr auto WHITE = RGB{255};
constexpr auto BLACK = RGB{0};
constexpr auto RED = RGB{255, 70, 70};
constexpr auto GREEN = RGB{70, 255, 70};
constexpr auto BLUE = RGB{70, 70, 255};

class FrameBufferCanvas {
public:
  FrameBufferCanvas(unsigned width, unsigned height);
  [[nodiscard]] unsigned width() const;
  [[nodiscard]] unsigned height() const;
  void set(unsigned x, unsigned y, RGB color);
  void drawLine(
      int ax, int ay, int bx, int by, int thickness, RGB color);
  void drawLine(const glm::vec2& start, const glm::vec2& end, int thickness,
                RGB color);
  void drawRect(unsigned centerX, unsigned centerY, unsigned w, unsigned h,
                RGB color);
  void drawRect(const glm::vec2& center, unsigned w, unsigned h, RGB color);
  void drawBezier(const glm::vec2& startPt,
                  const glm::vec2& controlPt,
                  const glm::vec2& endPt,
                  int thickness,
                  RGB color);

  void renderGlyph(
      const glm::vec2& startPt,
      const Glyph& glyph, RGB color);
  void writePngFile(const char* fileName) const;

private:
  unsigned width_;
  unsigned height_;
  std::unique_ptr<RGB[]> framebuffer;
};


#endif //PETITE_TRUETYPE_RENDERER_FRAMEBUFFERCANVAS_H