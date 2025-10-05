//
// Created by Yoh Yamasaki on 30/09/2025.
//

#include "FrameBufferCanvas.h"
#include <stb_image_write.h>
#include <cassert>
#include <memory>
#include <glm/geometric.hpp>

#include "GlyphComponent.h"


FrameBufferCanvas::FrameBufferCanvas(const unsigned width,
                                     const unsigned height) :
  width_(width), height_(height) {
  framebuffer = std::make_unique<RGB[]>(width * height);
  shiftYVec = glm::vec2(0, height);
  for (int i = 0; i < width * height; ++i) framebuffer[i] = BLACK;
}

unsigned FrameBufferCanvas::width() const {
  return width_;
}

unsigned FrameBufferCanvas::height() const {
  return height_;
}

void FrameBufferCanvas::set(const int x,
                            const int y,
                            const RGB color) {
  if (x < 0 || y < 0) return;
  if (static_cast<unsigned>(x) >= width_ || static_cast<unsigned>(y) >= height_)
    return;
  const std::size_t i = static_cast<std::size_t>(y) * static_cast<std::size_t>(
                          width_) + static_cast<std::size_t>(x);
  framebuffer[i] = color;
}

void FrameBufferCanvas::drawLine(int ax, int ay, int bx, int by,
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
      drawRect(static_cast<int>(y), x, thickness, thickness, color);
    } else {
      drawRect(x, static_cast<int>(y), thickness, thickness, color);
    }
    y += static_cast<float>(by - ay) / static_cast<float>(bx - ax);
  }
}

void FrameBufferCanvas::drawLine(const glm::vec2& start,
                                 const glm::vec2& end,
                                 const int thickness,
                                 const RGB color) {
  drawLine(static_cast<int>(std::lround(start.x)),
           static_cast<int>(std::lround(start.y)),
           static_cast<int>(std::lround(end.x)),
           static_cast<int>(std::lround(end.y)),
           thickness, color);
}


void FrameBufferCanvas::drawRect(const int centerX,
                                 const int centerY,
                                 const unsigned w,
                                 const unsigned h,
                                 const RGB color) {
  // compute integer bounds
  const int halfW = static_cast<int>(w) / 2;
  const int halfH = static_cast<int>(h) / 2;
  int left = centerX - halfW;
  int top = centerY - halfH;
  int right = left + static_cast<int>(w); // exclusive
  int bottom = top + static_cast<int>(h); // exclusive

  // clip to framebuffer
  if (right <= 0 || bottom <= 0) return; // fully left or above
  if (left >= static_cast<int>(width_) || top >= static_cast<int>(height_))
    return; // fully right or below

  left = std::max(left, 0);
  top = std::max(top, 0);
  right = std::min(right, static_cast<int>(width_));
  bottom = std::min(bottom, static_cast<int>(height_));

  for (int yy = top; yy < bottom; ++yy) {
    for (int xx = left; xx < right; ++xx) {
      set(xx, yy, color);
    }
  }
}

void FrameBufferCanvas::drawRect(const glm::vec2& center,
                                 const unsigned w,
                                 const unsigned h,
                                 const RGB color) {
  const int cx = static_cast<int>(std::lround(center.x));
  const int cy = static_cast<int>(std::lround(center.y));
  drawRect(cx, cy, w, h, color);
}

void FrameBufferCanvas::renderGlyph(const glm::vec2& startPt,
                                    const Glyph& glyph,
                                    const RGB color) {
  for (const auto& c : glyph.getComponents()) {
    uint16_t contourStartPt = 0;
    const auto n = c.getNumOfVertices();
    const auto coordinates = c.getCoordinates();
    const auto endPtsOfContours = c.getEndPtsOfContours();
    const auto ptsOnCurve = c.getPtsOnCurve();

    glm::vec2 prevPt = startPt;
    for (int i = 0; i < n; ++i) {
      const auto isEndPt = endPtsOfContours.contains(i);
      const auto nextIdx = isEndPt ? contourStartPt : (i + 1) % n;
      const auto isOnCurve = ptsOnCurve.contains(i);
      const auto isNextOnCurve = ptsOnCurve.contains(nextIdx);
      // Convert coordinate system from bottom-up to top-down
      auto currentPt = shiftYVec + flipYMat * (coordinates[i] + startPt);
      auto nextPt = shiftYVec + flipYMat * (coordinates[nextIdx] + startPt);

      if (isOnCurve) {
        if (isNextOnCurve) {
          drawLine(currentPt, nextPt, 5, color);
        }
        prevPt = currentPt;
      } else {
        // Control point
        if (!isNextOnCurve) {
          // Calculate the implicit next "on-curve" from the middle point of next control point
          nextPt = (currentPt + nextPt);
          nextPt /= 2;
        }
        drawBezier(prevPt, currentPt, nextPt, 5, color);
        // Draw implicit next on-curve point
        drawRect(nextPt, 20, 20, BLUE);
        prevPt = nextPt;
      }
      drawRect(currentPt, 20, 20, isOnCurve ? WHITE : GREEN);
      if (isEndPt) contourStartPt = i + 1;
    }
  }
}

void FrameBufferCanvas::writePngFile(const char* fileName) const {
  // TrueType uses bottom-to-top coordinate so we need to vertically flip the image
  stbi_write_png(fileName, static_cast<int>(width_),
                 static_cast<int>(height_),
                 3,
                 framebuffer.get(), 0);
}

void FrameBufferCanvas::drawBezier(const glm::vec2& startPt,
                                   const glm::vec2& controlPt,
                                   const glm::vec2& endPt,
                                   const int thickness,
                                   const RGB color) {
  // approximate curve length
  const float approxLen = glm::length(controlPt - startPt) + glm::length(
                              endPt - controlPt);
  const int res = std::max(1, static_cast<int>(std::ceil(approxLen)));
  glm::vec2 last = startPt;
  for (int i = 1; i <= res; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(res);
    glm::vec2 cur = quadBezierLerp(startPt, controlPt, endPt, t);
    drawLine(last, cur, thickness, color);
    last = cur;
  }
}

