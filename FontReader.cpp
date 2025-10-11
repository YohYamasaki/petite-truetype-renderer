#include "FontReader.h"

#include <cmath>
#include <iostream>
#include <glm/glm.hpp>

#include "FrameBufferCanvas.h"
#include "utils/Bit.h"
#include "utils/Geometry.h"
#include "utils/Unicode.h"

FontReader::FontReader(const std::string& path) {
  ifs.open(path, std::ios::binary);
  if (!ifs) {
    std::cerr << "failed to open file\n";
  }

  // read header
  skipBytes(sizeof(uint32_t)); // skip sfntVersion
  const uint16_t numTables = readUint16();
  // skip searchRange, entrySelector, rangeShift
  skipBytes(sizeof(uint16_t) * 3);

  // read directory table
  for (int i = 0; i < numTables; ++i) {
    constexpr unsigned bytesLength = 4;
    char bytes[bytesLength + 1];
    ifs.read(bytes, bytesLength);
    bytes[bytesLength] = '\0';
    const auto tag = std::string(bytes);
    const auto checkSum = readUint32();
    const auto offset = readUint32();
    const auto length = readUint32();
    directory[tag] = {checkSum, offset, length};
  }
  if (!directory.contains("glyf")) {
    throw std::runtime_error("Could not find glyf table");
  };

  // Load glyph related tables
  loadGlyphOffsetsMap();
  loadUnicodeToGlyphCodeMap();
  loadGlyphMetricsMap();
}

void FontReader::skipBytes(const unsigned bytes) {
  ifs.seekg(bytes, std::ios::cur);
}

void FontReader::jumpTo(const unsigned byteOffset) {
  ifs.seekg(byteOffset, std::ios::beg);
}

template <typename T>
T FontReader::readBeOrThrow() {
  static_assert(std::is_integral_v<T>, "T must be integral");

  uint32_t acc = 0;
  for (std::size_t i = 0; i < sizeof(T); ++i) {
    const int c = ifs.get();
    if (c == EOF) throw std::runtime_error("unexpected EOF");
    acc = (acc << 8) | static_cast<unsigned char>(c);
  }

  if constexpr (std::is_signed_v<T>) {
    const auto bits = static_cast<unsigned>(sizeof(T) * 8);
    const uint32_t sign_mask = (static_cast<uint32_t>(1) << (bits - 1));
    if (acc & sign_mask) {
      const uint64_t two_pow = static_cast<uint64_t>(1) << bits;
      const int64_t signed_val =
          static_cast<int64_t>(acc) - static_cast<int64_t>(two_pow);
      return static_cast<T>(signed_val);
    }
    return static_cast<T>(acc);
  } else {
    return static_cast<T>(acc);
  }
}

uint8_t FontReader::readUint8() { return readBeOrThrow<uint8_t>(); }
uint16_t FontReader::readUint16() { return readBeOrThrow<uint16_t>(); }
uint32_t FontReader::readUint32() { return readBeOrThrow<uint32_t>(); }
int8_t FontReader::readInt8() { return readBeOrThrow<int8_t>(); }
int16_t FontReader::readInt16() { return readBeOrThrow<int16_t>(); }
int32_t FontReader::readInt32() { return readBeOrThrow<int32_t>(); }

float FontReader::readF2Dot14() {
  const auto raw = readBeOrThrow<int16_t>();
  return static_cast<float>(raw) / static_cast<float>(1 << 14);
}

void FontReader::loadGlyphOffsetsMap() {
  jumpTo(directory["maxp"].offset + 4);
  const int numGlyphs = readUint16();

  jumpTo(directory["head"].offset);
  skipBytes(50); // skip until indexToLocFormat
  const auto isTwoByte = readInt16() == 0;
  const auto glyphTableOffset = directory["glyf"].offset;
  const auto locationTableOffset = directory["loca"].offset;

  jumpTo(locationTableOffset);
  for (int i = 0; i < numGlyphs; ++i) {
    const auto glyphOffset = isTwoByte ? readUint16() * 2u : readUint32();
    glyphCodeToOffset[i] = glyphTableOffset + glyphOffset;
  }
  // Remove offset for empty glyphs
  for (int i = 1; i < numGlyphs; ++i) {
    if (glyphCodeToOffset[i] == glyphCodeToOffset[i - 1]) {
      glyphCodeToOffset[i - 1] = 0;
    }
  }
}

void FontReader::loadGlyphMetricsMap() {
  jumpTo(directory["hhea"].offset);
  skipBytes(34); // Skip to numOfLongHorMetrics
  const u_int16_t numOfMetrics = readUint16();
  jumpTo(directory["hmtx"].offset);
  for (int i = 0; i < numOfMetrics; ++i) {
    const u_int16_t width = readUint16();
    const int16_t lsb = readInt16();
    glyphMetric[i] = Metric{width, lsb};
  }
}

void FontReader::loadUnicodeToGlyphCodeMap() {
  const auto cmapOffset = directory["cmap"].offset;
  jumpTo(cmapOffset);
  skipBytes(2); // skip version
  const uint16_t numSubtables = readUint16();

  u_int32_t subtableOffset = -1;
  for (int i = 0; i < numSubtables; ++i) {
    const auto platform = readUint16();
    const auto encoding = readUint16();
    const auto offset = readUint32();
    // Only supports Unicode with format12 for now
    // Ideally format4 should be supported for a fallback
    if (platform == 0 && encoding == 4) subtableOffset = offset;
  }

  if (subtableOffset == -1) {
    throw std::runtime_error("Not supported format");
  }
  jumpTo(cmapOffset + subtableOffset);

  // read subtable header to detect format
  const uint16_t format = readUint16();
  if (format == 12) {
    skipBytes(10); // skip reserved, length, language
    const uint32_t nGroups = readUint32();

    for (int i = 0; i < nGroups; ++i) {
      const auto startCharCode = readUint32();
      const auto endCharCode = readUint32();
      const auto startGlyphCode = readUint32();
      unicodeToGlyphCode[startCharCode] = startGlyphCode;
      const auto diff = (endCharCode - startCharCode);
      for (int j = 1; j <= diff; ++j) {
        unicodeToGlyphCode[startCharCode + j] = startGlyphCode + j;
      }
    }
  } else {
    throw std::runtime_error("Not supported format");
  }
}

std::pair<std::vector<Glyph>, int> FontReader::getGlyphs(
    std::vector<uint32_t> cps,
    const float scale) {
  // Get glyph data
  int width = 0;
  std::vector<Glyph> glyphs(0);
  for (const auto& cp : cps) {
    const auto glyph = getGlyph(cp);
    glyphs.emplace_back(glyph);
    width += glyph.getMetric().advanceWidth * scale;
  }

  return {glyphs, width};
}

Glyph FontReader::getGlyph(const uint32_t cp) {
  if (!unicodeToGlyphCode.contains(cp)) {
    throw std::runtime_error("Glyph not found");
  }
  return getGlyphByCode(unicodeToGlyphCode[cp]);
}

GlyphHeader FontReader::readGlyphHeader(const uint16_t glyphCode) {
  const auto offset = glyphCodeToOffset[glyphCode];
  if (offset == 0) {
    return GlyphHeader{0, BoundingRect{0, 0, 0, 0}};
  }

  jumpTo(offset);
  // Read glyph description
  const auto numOfContours = readInt16(); // number of contours
  BoundingRect boundingRect;
  boundingRect.xMin = readInt16();
  boundingRect.yMin = readInt16();
  boundingRect.xMax = readInt16();
  boundingRect.yMax = readInt16();

  return GlyphHeader{numOfContours, boundingRect};
}

Glyph FontReader::getGlyphByCode(const uint16_t glyphCode) {
  const auto [numOfContours, boundingRect] = readGlyphHeader(glyphCode);
  Glyph glyph;
  const auto metric = glyphMetric[glyphCode];
  if (numOfContours == 0) {
    // No glyph needed i.e. space
    glyph = Glyph::EmptyGlyph(metric);
  } else if (numOfContours > 0) {
    // Read simple glyphs
    glyph = Glyph({getGlyphComponent(numOfContours, boundingRect)},
                  metric);
  } else {
    glyph = getCompoundGlyph();
  }
  return glyph;
}

std::vector<GlyphComponent> FontReader::getCompoundSubComponents(
    const uint16_t glyphCode,
    const glm::mat3& affineMat) {
  const auto [numOfContours, boundingRect] = readGlyphHeader(glyphCode);
  std::vector<GlyphComponent> components;
  if (numOfContours > 0) {
    // Simple
    components.emplace_back(
        getGlyphComponent(numOfContours, boundingRect, affineMat));
  } else {
    // Compound: Compound glyph can have nested Compound glyphs
    const auto subComponents = getCompoundGlyph().getComponents();
    components.insert(components.begin(), subComponents.begin(),
                      subComponents.end());
  }
  return components;
}

GlyphComponent FontReader::getGlyphComponent(
    const int16_t numOfContours,
    const BoundingRect boundingRect,
    const glm::mat3& affineMat) {
  std::unordered_set<uint16_t> endPtsOfContours;

  uint16_t numOfVertices = 1;
  for (int i = 0; i < numOfContours; ++i) {
    const auto point = readUint16();
    numOfVertices = std::max(numOfVertices, static_cast<uint16_t>(point + 1));
    endPtsOfContours.insert(point);
  }

  // Skip instructions
  skipBytes(readUint16());

  std::vector<uint8_t> allFlags(numOfVertices, 0);
  std::unordered_set<uint16_t> ptsOnCurve;

  uint16_t idx = 0;
  while (idx < numOfVertices) {
    const uint8_t f = readUint8();
    allFlags[idx] = f;
    if (Bit::isFlagSet(f, 0)) {
      //ON_CURVE_POINT
      ptsOnCurve.insert(idx);
    }

    if (Bit::isFlagSet(f, 3)) {
      // REPEAT_FLAG
      const uint8_t repeat = readUint8(); // number of repetitions
      // fill repeats
      for (uint16_t r = 1; r <= repeat; ++r) allFlags[idx + r] = f;
      idx += static_cast<uint16_t>(repeat) + 1;
    } else {
      ++idx;
    }
  }

  const std::vector<int> xCoordinates = getGlyphCoordinates(
      numOfVertices, allFlags, true);
  const std::vector<int> yCoordinates = getGlyphCoordinates(
      numOfVertices, allFlags, false);
  std::vector<glm::vec2> coordinates;
  for (int i = 0; i < numOfVertices; ++i) {
    const auto coord = affineMat * glm::vec3(xCoordinates[i], yCoordinates[i],
                                             1);
    coordinates.emplace_back(coord.x, coord.y);
  }

  return GlyphComponent{numOfVertices, endPtsOfContours, ptsOnCurve,
                        boundingRect, coordinates};
}

std::vector<int> FontReader::getGlyphCoordinates(const uint16_t& n,
                                                 const std::vector<uint8_t>&
                                                 flags,
                                                 const bool isX) {
  std::vector coordinates(n, 0);
  for (int i = 0; i < n; ++i) {
    const auto f = flags[i];
    const auto isShort = Bit::isFlagSet(f, isX ? 1 : 2);
    int pos = coordinates[i == 0 ? 0 : i - 1];

    if (isShort) {
      const auto isPositive = Bit::isFlagSet(f, isX ? 4 : 5);
      const auto offset = isPositive
                            ? readUint8()
                            : -1 * static_cast<int>(readUint8());
      pos += offset;
    } else {
      if (!Bit::isFlagSet(f, isX ? 4 : 5)) {
        pos += readInt16();
      };
    }

    coordinates[i] = pos;
  }
  return coordinates;
}

Glyph FontReader::getCompoundGlyph() {
  std::vector<GlyphComponent> components;
  uint16_t flags;
  Metric metric{};
  do {
    flags = readUint16();
    const uint16_t glyphCode = readUint16();
    // read flags
    const bool isWord = Bit::isFlagSet(flags, 0);
    const bool isXyValue = Bit::isFlagSet(flags, 1); // not implemented
    const bool roundXyGrid = Bit::isFlagSet(flags, 2); // not implemented
    const bool hasScale = Bit::isFlagSet(flags, 3);
    const bool hasXScale = Bit::isFlagSet(flags, 6);
    const bool hasTwoByTwo = Bit::isFlagSet(flags, 7);
    const bool hasInstruction = Bit::isFlagSet(flags, 8); // not implemented
    const bool useMetrics = Bit::isFlagSet(flags, 9);
    const bool overlap = Bit::isFlagSet(flags, 10); // not implemented

    // read arguments (arg1,arg2) either words or bytes (signed)
    int32_t arg1 = 0, arg2 = 0;
    if (isWord) {
      arg1 = readInt16();
      arg2 = readInt16();
    } else {
      arg1 = static_cast<int32_t>(static_cast<unsigned char>(readInt8()));
      arg2 = static_cast<int32_t>(static_cast<unsigned char>(readInt8()));
    }
    // interpret arg1/arg2 later: XY values or point indices
    const int32_t e_raw = arg1;
    const int32_t f_raw = arg2;

    // read transform values only when indicated
    double a = 1.0, b = 0.0, c = 0.0, d = 1.0;
    if (hasScale) {
      const double s = readF2Dot14(); // single
      a = d = s;
      b = c = 0.0;
    } else if (hasXScale) {
      a = readF2Dot14();
      d = readF2Dot14();
      b = c = 0.0;
    } else if (hasTwoByTwo) {
      a = readF2Dot14();
      b = readF2Dot14();
      c = readF2Dot14();
      d = readF2Dot14();
    } // else identity

    // Normalization factors
    constexpr float eps = 33.0 / 65536.0;
    float m = static_cast<float>(std::max(std::fabs(a), std::fabs(b)));
    m = std::fabs(a) - std::fabs(c) <= eps ? 2 * m : m;
    float n = static_cast<float>(std::max(std::fabs(c), std::fabs(d)));
    n = std::fabs(b) - std::fabs(d) <= eps ? 2 * n : n;
    const auto e = static_cast<float>(e_raw);
    const auto f = static_cast<float>(f_raw);

    // The affine transformation matrix, looks like this:
    // [a, c, me]
    // [b, d, nf]
    // [0, 0,  1]
    const auto affineMat = glm::mat3(a, b, 0, c, d, 0, m * e, n * f, 1);

    // Needs to save the current reader pos since getGlyph jumps to a certain offset pos
    const auto currentOffset = ifs.tellg();
    const auto subComponents =
        getCompoundSubComponents(glyphCode, affineMat);
    if (subComponents.empty()) {
      throw std::runtime_error(
          "FontReader: Something went wrong with loading component glyphs");
    }
    components.insert(components.end(), subComponents.begin(),
                      subComponents.end());

    if (useMetrics) {
      metric = glyphMetric[glyphCode];
    }
    // Reload the prev reader pos for the next loop
    jumpTo(currentOffset);
  } while (Bit::isFlagSet(flags, 5)); // MORE_COMPONENTS

  return Glyph(components, metric);
}

FontMetric FontReader::getFontMetric() {
  jumpTo(directory["hhea"].offset);
  skipBytes(4);
  const auto ascent = readInt16();
  const auto descent = readInt16();
  return FontMetric{ascent, descent};
}