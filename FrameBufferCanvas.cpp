#include "FrameBufferCanvas.h"
#include <stb_image_write.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <span>
#include <unordered_map>
#include <glm/glm.hpp>

#include "GlyphComponent.h"
#include "utils/Debug.h"


FrameBufferCanvas::FrameBufferCanvas(const int width_,
                                     const int height_) :
  width(width_), height(height_) {
  framebuffer = std::make_unique<RGB[]>(width * height);
  transformMat = glm::mat3(1, 0, 0, 0, -1, 0, 0, 0, 1);
  for (int i = 0; i < width * height; ++i) framebuffer[i] = BLACK;
}

void FrameBufferCanvas::set(const int x,
                            const int y,
                            const RGB color) {
  if (x < 0 || y < 0) return;
  if (x >= width || y >= height) return;
  const std::size_t i = static_cast<std::size_t>(y) * static_cast<std::size_t>(
                          width) + static_cast<std::size_t>(x);
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
                                 const int rectWidth,
                                 const int rectHeight,
                                 const RGB color) {
  // compute integer bounds
  const int halfW = rectWidth / 2;
  const int halfH = rectHeight / 2;
  int left = centerX - halfW;
  int top = centerY - halfH;
  int right = left + rectWidth; // exclusive
  int bottom = top + rectHeight; // exclusive

  // clip to framebuffer
  if (right <= 0 || bottom <= 0) return; // fully left or above
  if (left >= width || top >= height)
    return; // fully right or below

  left = std::max(left, 0);
  top = std::max(top, 0);
  right = std::min(right, width);
  bottom = std::min(bottom, height);

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

void FrameBufferCanvas::renderGlyphOutline(const Glyph& glyph,
                                           const RGB color,
                                           const int startX,
                                           const int thickness) {
  for (const auto& c : glyph.getComponents()) {
    const auto rectSize = thickness * 2;
    uint16_t contourStartPt = 0;
    const auto n = c.getNumOfVertices();
    const auto coordinates = c.getCoordinates();
    const auto endPtsOfContours = c.getEndPtsOfContours();
    const auto ptsOnCurve = c.getPtsOnCurve();

    transformMat[2][0] = startX;

    // Cache of point vectors of vertices
    std::unordered_map<int, glm::vec2> points;
    for (int i = 0; i < n; ++i) {
      // Convert coordinate system from bottom-up to top-down
      points[i] = transformVec2(scale * transformMat, coordinates[i]);
    }

    auto prevPt = glm::vec2(0.f);
    for (int i = 0; i < n; ++i) {
      const auto isEndPt = endPtsOfContours.contains(i);
      const auto nextIdx = isEndPt ? contourStartPt : (i + 1) % n;
      const auto isOnCurve = ptsOnCurve.contains(i);
      const auto isNextOnCurve = ptsOnCurve.contains(nextIdx);
      // Convert coordinate system from bottom-up to top-down
      auto currentPt = points[i];
      auto nextPt = points[nextIdx];

      if (isOnCurve) {
        if (isNextOnCurve) {
          drawLine(currentPt, nextPt, thickness, color);
        }
        prevPt = currentPt;
      } else {
        // Control point
        if (!isNextOnCurve) {
          // Calculate the implicit next "on-curve" from the middle point of next control point
          nextPt = (currentPt + nextPt);
          nextPt /= 2;
        }
        drawBezier(prevPt, currentPt, nextPt, thickness, color);
        // Draw implicit next on-curve point
        drawRect(nextPt, rectSize, rectSize, BLUE);
        prevPt = nextPt;
      }
      drawRect(currentPt, rectSize, rectSize, isOnCurve ? RED : GREEN);
      if (isEndPt) contourStartPt = i + 1;
    }
  }
}

void FrameBufferCanvas::renderGlyphByEvenOdd(const Glyph& glyph,
                                             const RGB color,
                                             const int startX) {
  const auto start = std::chrono::high_resolution_clock::now();

  // Loop for each glyph components
  for (const auto& c : glyph.getComponents()) {
    const auto n = c.getNumOfVertices();
    const auto coordinates = c.getCoordinates();
    const auto endPtsOfContours = c.getEndPtsOfContours();
    const auto ptsOnCurve = c.getPtsOnCurve();

    transformMat[2][0] = startX;

    // Cache of point vectors of vertices
    std::unordered_map<int, glm::vec2> points;
    for (int i = 0; i < n; ++i) {
      // Convert coordinate system from bottom-up to top-down
      points[i] = transformVec2(scale * transformMat, coordinates[i]);
    }

    // Loop for each scanline
    auto rayStart = glm::vec2(0, 0);
    auto rayEnd = glm::vec2(width, 0);
    for (int y = 0; y < height; ++y) {
      rayStart.y = rayEnd.y = static_cast<float>(y);
      std::vector<int> intersections;
      uint16_t contourStartPt = 0;
      auto prevPt = glm::vec2(0.f);
      // Loop for each vertex
      for (int i = 0; i < n; ++i) {
        const auto isEndPt = endPtsOfContours.contains(i);
        const auto nextIdx = isEndPt ? contourStartPt : (i + 1) % n;
        const auto isOnCurve = ptsOnCurve.contains(i);
        const auto isNextOnCurve = ptsOnCurve.contains(nextIdx);
        auto currentPt = points[i];
        auto nextPt = points[nextIdx];

        if (isOnCurve) {
          if (isNextOnCurve) {
            // on curve - on curve: Straight line
            const auto s = segmentsIntersect(
                currentPt, nextPt, rayStart, rayEnd);
            const auto minY = static_cast<int>(std::min(currentPt.y, nextPt.y));
            if (s && minY < y) {
              intersections.emplace_back(static_cast<int>(s.value().x));
            }
          }
          prevPt = currentPt;
        } else {
          // not on curve: Control point
          if (!isNextOnCurve) {
            // Calculate the implicit next "on-curve" from the middle point of next control point
            nextPt = (currentPt + nextPt);
            nextPt /= 2;
          }
          const auto ss = segmentQuadBezierIntersect(
              prevPt,
              currentPt,
              nextPt,
              rayStart,
              rayEnd
              );
          const auto minY = getBezierMinY(prevPt, currentPt, nextPt);

          if (ss.size() == 2 &&
              !isBezierConvexUpwards(prevPt, currentPt, nextPt)) {
            if (y == static_cast<int>(prevPt.x) || y == static_cast<int>(nextPt.
                  x)) {
              if (static_cast<int>(ss[0].x) == static_cast<int>(prevPt.x)) {
                intersections.emplace_back(static_cast<int>(ss[1].x));
              } else {
                intersections.emplace_back(static_cast<int>(ss[0].x));
              }
            }
          } else if (minY < y) {
            for (const auto s : ss) {
              intersections.emplace_back(static_cast<int>(s.x));
            }
          }
          prevPt = nextPt;
        }
        if (isEndPt) contourStartPt = i + 1;
      }

      std::sort(intersections.begin(), intersections.end());
      int cnt = 0;
      int prevX = 0;
      for (const auto x : intersections) {
        if (cnt % 2 == 1) {
          drawLine(prevX, y, x, y, 1, color);
        }
        cnt++;
        prevX = x;
      }
    }
  }
  const auto end = std::chrono::high_resolution_clock::now();
  const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end - start);
  std::wcout << "Evenodd glyph rendering tooks: " << duration.count() <<
      " ms\n";
}

void FrameBufferCanvas::setGlyphBaseline(const int baseline) {
  transformMat[2][1] += baseline;
}

void FrameBufferCanvas::renderGlyphs(const std::vector<Glyph>& glyphs) {
  int xPos = 0;
  for (const auto& glyph : glyphs) {
    renderGlyphByNonZero(glyph, RGB{255}, xPos);
    xPos += glyph.getMetric().advanceWidth;
  }
}

void FrameBufferCanvas::renderGlyphByNonZero(const Glyph& glyph,
                                             const RGB color,
                                             const int startX) {
  const auto start = std::chrono::high_resolution_clock::now();

  // Loop for each glyph components
  for (const auto& c : glyph.getComponents()) {
    const auto n = c.getNumOfVertices();
    const auto coordinates = c.getCoordinates();
    const auto endPtsOfContours = c.getEndPtsOfContours();
    const auto ptsOnCurve = c.getPtsOnCurve();
    const auto rect = c.getBoundingRect();

    transformMat[2][0] = startX;

    // Cache of point vectors of vertices
    std::unordered_map<int, glm::vec2> points;
    for (int i = 0; i < n; ++i) {
      // Convert coordinate system from bottom-up to top-down
      points[i] = transformVec2(scale * transformMat, coordinates[i]);
    }
    auto rayStart = glm::vec2(0, 0);
    auto rayEnd = glm::vec2(width, 0);

    // Transform the coordinate of the bounding rect
    const auto top = transformVec2(
        scale * transformMat, glm::vec2(0, rect.yMax)).y;
    const auto bottom = transformVec2(
        scale * transformMat, glm::vec2(0, rect.yMin)).y;
    for (int y = top; y < bottom; ++y) {
      rayStart.y = rayEnd.y = static_cast<float>(y);
      std::vector<Intersection> intersections;
      uint16_t contourStartPt = 0;
      auto prevPt = glm::vec2{0, 0};
      // Loop for each vertex
      for (int i = 0; i < n; ++i) {
        const auto isEndPt = endPtsOfContours.contains(i);
        const auto nextIdx = isEndPt ? contourStartPt : (i + 1) % n;
        const auto isOnCurve = ptsOnCurve.contains(i);
        const auto isNextOnCurve = ptsOnCurve.contains(nextIdx);
        auto currentPt = points[i];
        auto nextPt = points[nextIdx];

        if (isOnCurve) {
          if (isNextOnCurve) {
            // on curve - on curve: Straight line
            const auto s = segmentsIntersect(
                currentPt, nextPt, rayStart, rayEnd);
            const auto minY = static_cast<int>(std::min(currentPt.y, nextPt.y));
            if (s && minY < y) {
              intersections.emplace_back(
                  static_cast<int>(s.value().x),
                  currentPt.y > nextPt.y
                  );
            }
          }
          prevPt = currentPt;
        } else {
          // not on curve: Control point
          if (!isNextOnCurve) {
            // Calculate the implicit next "on-curve" from the middle point of next control point
            nextPt = (currentPt + nextPt);
            nextPt /= 2;
          }
          // bezier line
          const auto ss = segmentQuadBezierIntersect(
              prevPt,
              currentPt,
              nextPt,
              rayStart,
              rayEnd
              );

          const auto minY = getBezierMinY(prevPt, currentPt, nextPt);
          if (minY < y) {
            if (ss.size() == 1) {
              intersections.emplace_back(static_cast<int>(ss[0].x),
                                         prevPt.y > nextPt.y);
            } else if (ss.size() == 2) {
              auto nearestX = [&](const glm::vec2& pt) -> int {
                return static_cast<int>((std::fabs(pt.x - ss[0].x) <
                                         std::fabs(pt.x - ss[1].x))
                                          ? ss[0].x
                                          : ss[1].x);
              };
              const int closeXtoCur = nearestX(prevPt);
              const int closeXtoNxt = nearestX(nextPt);
              const auto convexUpwards = isBezierConvexUpwards(
                  prevPt, currentPt, nextPt);
              intersections.emplace_back(closeXtoCur, convexUpwards);
              intersections.emplace_back(closeXtoNxt, !convexUpwards);
            }
          }
          prevPt = nextPt;
        }
        if (isEndPt) contourStartPt = i + 1;
      }

      std::sort(intersections.begin(), intersections.end(),
                [](const Intersection& a, const Intersection& b) {
                  return a.x < b.x;
                });
      int prevCount = 0;
      int prevX = 0;
      int count = 0;
      for (const auto& [x, isUpward] : intersections) {
        isUpward ? ++count : --count;
        if (prevCount > 0 && count >= 0) {
          drawLine(prevX, y, x, y, 1, color);
        }
        prevCount = count;
        prevX = x;
      }
    }
  }

  const auto end = std::chrono::high_resolution_clock::now();
  const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end - start);
  std::wcout << "NonZero glyph rendering tooks: " << duration.count() <<
      " ms\n";
}

void FrameBufferCanvas::writePngFile(const char* fileName) const {
  // TrueType uses bottom-to-top coordinate so we need to vertically flip the image
  stbi_write_png(fileName, width, height, 3,
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

void FrameBufferCanvas::setScale(float s) {
  scale = s;
}