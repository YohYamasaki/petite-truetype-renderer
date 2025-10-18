#pragma once
#ifndef FRAMEBUFFERCANVAS_H
#define FRAMEBUFFERCANVAS_H
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <memory>
#include <glm/glm.hpp>

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

  bool operator==(const RGB& a) const {
    return r == a.r && g == a.g && b == a.b;
  }
};

struct Intersection {
  int x;
  bool isUpward;
};

constexpr auto WHITE = RGB{255};
constexpr auto BLACK = RGB{0};
constexpr auto RED = RGB{255, 70, 70};
constexpr auto GREEN = RGB{70, 255, 70};
constexpr auto BLUE = RGB{70, 70, 255};

constexpr auto WIDTH = 1500;
constexpr auto HEIGHT = 1500;

class FrameBufferCanvas {
public:
  explicit FrameBufferCanvas(int width_ = WIDTH, int height_ = HEIGHT);
  /**
   * Set one pixel with the given color.
   * @param x Target x position
   * @param y Target y position
   * @param color Fill color
   */
  void set(int x, int y, RGB color);
  /**
   * Draw a line from point A to B.
   * @param ax Start point's x position
   * @param ay Start point's y position
   * @param bx End point's x position
   * @param by End point's y position
   * @param thickness Line thickness
   * @param color Fill color
   */
  void drawLine(
      int ax, int ay, int bx, int by, int thickness, RGB color);
  /**
   * Draw a line from point A to B.
   * @param start Start point's vector
   * @param end End point's vector
   * @param thickness Line thickness
   * @param color Fill color
   */
  void drawLine(const glm::vec2& start, const glm::vec2& end, int thickness,
                RGB color);
  /**
   * Draw a rectangle.
   * @param centerX Rectangle center x position
   * @param centerY Rectangle center y position
   * @param rectWidth Rectangle width
   * @param rectHeight Rectangle height
   * @param color Fill color
   */
  void drawRect(int centerX, int centerY, int rectWidth, int rectHeight,
                RGB color);
  /**
   * Draw a rectangle.
   * @param center Rectangle center position vector
   * @param w Rectangle width
   * @param h Rectangle height
   * @param color Fill color
   */
  void drawRect(const glm::vec2& center, unsigned w, unsigned h, RGB color);
  /**
   * Draw a quadratic Bezier curve.
   * @param startPt Start point of the Bezier
   * @param controlPt Control point of the Bezier
   * @param endPt End point of the Bezier
   * @param thickness Line thickness
   * @param color Fill color
   */
  void drawBezier(const glm::vec2& startPt,
                  const glm::vec2& controlPt,
                  const glm::vec2& endPt,
                  int thickness,
                  RGB color);
  /**
   * Set the baseline of glyph rendering.
   * @param baseline y position of glyph baseline
   */
  void setGlyphBaseline(int baseline);
  /**
   * Render glyphs by using non-zero rule.
   * @param glyphs Vector of glyphs to render
   */
  void renderGlyphs(const std::vector<Glyph>& glyphs);
  /**
   * Render an outline of the target glyph.
   * @param glyph Glyph
   * @param color Fill color
   * @param startX
   * @param thickness
   */
  void renderGlyphOutline(const Glyph& glyph, RGB color, int startX,
                          int thickness);
  /**
   * Render a target glyph by even–odd rule.
   * @param glyph Glyph
   * @param color Fill color
   * @param startX
   */
  void renderGlyphByEvenOdd(const Glyph& glyph, RGB color, int startX);
  /**
   * Render a target glyph by non-zero rule.
   * @param glyph Glyph
   * @param color Fill color
   * @param startX
   */
  void renderGlyphByNonZero(const Glyph& glyph, RGB color, int startX);
  /**
   * Export the framebuffer to a png file.
   * @param fileName File name of the png file
   */
  void writePngFile(const char* fileName) const;
  void setScale(float s);

private:
  int width;
  int height;
  float scale;
  std::unique_ptr<RGB[]> framebuffer;
  glm::mat3 transformMat{};
};


#endif //FRAMEBUFFERCANVAS_H