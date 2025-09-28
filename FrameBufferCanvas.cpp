//
// Created by Yoh Yamasaki on 30/09/2025.
//

#include "FrameBufferCanvas.h"
#include <stb_image_write.h>
#include <cassert>
#include <memory>

#include "Glyph.h"

FrameBufferCanvas::FrameBufferCanvas(const unsigned int width,
                                     const unsigned int height) :
  width_(width), height_(height) {
  framebuffer = std::make_unique<RGB[]>(width * height);
  for (int i = 0; i < width * height; ++i) framebuffer[i] = RGB{0};
}

unsigned int FrameBufferCanvas::width() const {
  return width_;
}

unsigned int FrameBufferCanvas::height() const {
  return height_;
}

void FrameBufferCanvas::set(const unsigned int x,
                            const unsigned int y,
                            const RGB color) {
  const unsigned int i = width_ * y + x;
  assert(x < width_ && y < height_);
  framebuffer[i] = color;
}

void FrameBufferCanvas::line(int ax, int ay, int bx, int by,
                             const int thickness,
                             const RGB color) {
  const int w = std::abs(ax - bx);
  const int h = std::abs(ay - by);
  const bool isSteep = w < h;
  if (isSteep) {
    std::swap(ax, ay);
    std::swap(bx, by);
  }
  if (ax > bx) {
    std::swap(ax, bx);
    std::swap(ay, by);
  }
  auto y = static_cast<float>(ay);
  for (int x = ax; x <= bx; ++x) {
    if (isSteep) {
      rect(static_cast<int>(y), x, thickness, thickness, color);
    } else {
      rect(x, static_cast<int>(y), thickness, thickness, color);
    }
    y += static_cast<float>(by - ay) / static_cast<float>(bx - ax);
  }
}


void FrameBufferCanvas::rect(const unsigned int centerX,
                             const unsigned int centerY,
                             const unsigned int w,
                             const unsigned int h,
                             const RGB color) {
  assert(centerX + w/2 <= width_ && centerY + h/2 <= height_);
  for (unsigned int dy = 0; dy < h; ++dy) {
    for (unsigned int dx = 0; dx < w; ++dx) {
      set(centerX + dx-w/2, centerY + dy-h/2, color);
    }
  }
}

void FrameBufferCanvas::renderGlyph(const unsigned int startX,
                                    const unsigned int startY,
                                    const Glyph& glyph,
                                    const RGB color) {
  uint16_t contourStartPt = 0;
  const auto n = glyph.getNumOfVertices();
  const auto xCoordinates = glyph.getXCoordinates();
  const auto yCoordinates = glyph.getYCoordinates();
  const auto endPtsOfContours = glyph.getEndPtsOfContours();

  for (int i = 0; i < n; ++i) {
    const auto isEndPt = endPtsOfContours.contains(i);
    const auto targetIdx = isEndPt ? contourStartPt : (i + 1) % n;
    const auto x = xCoordinates[i] + startX;
    const auto y = yCoordinates[i] + startY;
    const auto targetX = xCoordinates[targetIdx] + startX;
    const auto targetY = yCoordinates[targetIdx] + startY;
    line(x, y, targetX, targetY, 5, color);
    rect(x, y, 20, 20, color);
    if (isEndPt) contourStartPt = i + 1;
  }
}

void FrameBufferCanvas::writePngFile(const char* fileName) const {
  // TrueType uses bottom-to-top coordinate so we need to vertically flip the image
  stbi_flip_vertically_on_write(1);
  stbi_write_png(fileName, static_cast<int>(width_),
                 static_cast<int>(height_),
                 3,
                 framebuffer.get(), 0);
}

