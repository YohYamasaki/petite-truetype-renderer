//
// Created by yoh on 28/09/25.
//

#include "FontReader.h"

#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include "utils/Bit.h"
#include "utils/Debug.h"
#include "utils/Geometry.h"
#include "utils/Unicode.h"

FontReader::FontReader(const std::string& path) {
  ifs.open(path, std::ios::binary);
  if (!ifs) {
    std::cerr << "failed to open file\n";
  }

  // read HEAD table
  skipBytes(sizeof(uint32_t)); // skip sfntVersion
  const uint16_t numTables = readUint16();
  // skip searchRange, entrySelector, rangeShift
  skipBytes(sizeof(uint16_t) * 3);

  // read directory table
  for (int i = 0; i < numTables; ++i) {
    const auto tag = readTag();
    const auto checkSum = readUint32();
    const auto offset = readUint32();
    const auto length = readUint32();
    directory[tag] = {checkSum, offset, length};
  }
  if (!directory.contains("glyf")) {
    throw std::runtime_error("Could not find glyf table");
  };
  loadGlyphOffsets();
  loadUnicodeToGlyphCodeTable();
}

void FontReader::skipBytes(const unsigned int bytes) {
  ifs.seekg(bytes, std::ios::cur);
}

void FontReader::goTo(const unsigned int targetByte) {
  ifs.seekg(targetByte, std::ios::beg);
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

std::string FontReader::readTag() {
  constexpr unsigned int bytesLength = 4;
  char b[bytesLength + 1];
  ifs.read(b, bytesLength);
  b[bytesLength] = '\0';
  std::string str(b);
  return str;
}


void FontReader::loadGlyphOffsets() {
  goTo(directory["maxp"].offset + 4);
  const int numGlyphs = readUint16();

  goTo(directory["head"].offset);
  skipBytes(50);
  const auto isTwoByte = readInt16() == 0;

  const auto locationTableOffset = directory["loca"].offset;
  const auto glyphTableOffset = directory["glyf"].offset;

  goTo(locationTableOffset);
  for (int i = 0; i < numGlyphs; ++i) {
    const auto glyphOffset = isTwoByte ? readUint16() * 2u : readUint32();
    glyphCodeToOffset[i] = glyphTableOffset + glyphOffset;
  }
}


/**
 * Load Unicode to Glyph code table. Glyph code is not the offset in ttf file,
 * the glyphCodeToOffset is there to retrieve it.
 * Only supports format 12 table for now.
 *
 * https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
 */
void FontReader::loadUnicodeToGlyphCodeTable() {
  const auto cmapOffset = directory["cmap"].offset;
  goTo(cmapOffset);
  skipBytes(2); // skip version
  const uint16_t numSubtables = readUint16();

  u_int32_t subtableOffset = -1;
  for (int i = 0; i < numSubtables; ++i) {
    const auto platform = readUint16();
    const auto encoding = readUint16();
    const auto offset = readUint32();
    // Only supports Unicode with format12 for now
    // format4 support is required for fallback
    if (platform == 0 && encoding == 4) subtableOffset = offset;
  }

  if (subtableOffset == -1) {
    throw std::runtime_error("Not supported format");
  }
  goTo(cmapOffset + subtableOffset);

  // read subtable header to detect format
  const uint16_t format = readUint16();
  if (format == 12) {
    readCmapFormat12();
  } else {
    throw std::runtime_error("Not supported format");
  }
}

void FontReader::readCmapFormat12() {
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
}

Glyph FontReader::getGlyph(const std::string& s) {
  const auto unicode = utf8ToCodepoints(s);
  return getGlyphByOffset(
      getGlyphOffsetByUnicode(static_cast<wchar_t>(unicode[0])));
}

Glyph FontReader::getGlyphByOffset(const uint32_t offset,
                                   const glm::mat3& affineMat) {
  goTo(offset);
  // Read glyph description
  const auto numOfContours = readInt16(); // number of contours

  BoundingRect boundingRect;
  boundingRect.xMin = readInt16();
  boundingRect.yMin = readInt16();
  boundingRect.xMax = readInt16();
  boundingRect.yMax = readInt16();

  // Read simple glyphs
  Glyph glyph;
  if (numOfContours > 0) {
    glyph = getSimpleGlyph(numOfContours, boundingRect, affineMat);
  } else {
    glyph = getCompoundGlyph();
  }
  return glyph;
}

/**
 * Load simple glyph. Used from getCompoundGlyph to get component glyphs.
 *
 * https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6glyf.html
 */
Glyph FontReader::getSimpleGlyph(const int16_t numOfContours,
                                 const BoundingRect boundingRect,
                                 const glm::mat3& affineMat) {
  std::set<uint16_t> endPtsOfContours;
  uint16_t numOfVertices = 1;
  for (int i = 0; i < numOfContours; ++i) {
    const auto point = readUint16();
    numOfVertices = std::max(numOfVertices, static_cast<uint16_t>(point + 1));
    endPtsOfContours.insert(point);
  }

  // Skip instructions
  skipBytes(readUint16());

  std::vector<uint8_t> flags(numOfVertices, 0);
  uint16_t idx = 0;
  while (idx < numOfVertices) {
    const uint8_t f = readUint8();
    flags[idx] = f;

    if (Bit::isFlagSet(f, 3)) {
      // repeat flag
      const uint8_t repeat = readUint8(); // number of repetitions
      // check bounds: idx + repeat must be < numOfVertices
      if (static_cast<uint32_t>(idx) + static_cast<uint32_t>(repeat) >=
          numOfVertices) {
        throw std::runtime_error(
            "FontReader: flags repeat runs past number of vertices");
      }
      // fill repeats
      for (uint16_t r = 1; r <= repeat; ++r) flags[idx + r] = f;
      idx += static_cast<uint16_t>(repeat) + 1;
    } else {
      ++idx;
    }
  }

  const std::vector<int> xCoordinates = readGlyphCoordinates(
      numOfVertices, flags, true);
  const std::vector<int> yCoordinates = readGlyphCoordinates(
      numOfVertices, flags, false);
  std::vector<glm::vec2> coordinates;
  for (int i = 0; i < numOfVertices; ++i) {
    const auto coord = affineMat * glm::vec3(xCoordinates[i], yCoordinates[i],
                                             1);
    coordinates.emplace_back(coord.x, coord.y);
  }

  GlyphComponent component{numOfVertices, endPtsOfContours, boundingRect,
                           coordinates};
  return Glyph({component});
}

/**
 * Load compound glyph.
 * ARGS_ARE_XY_VALUES, ROUND_XY_TO_GRID, WE_HAVE_INSTRUCTIONS, OVERLAP_COMPOUND
 * are not supported.
 * Not tested complex affine-transformation thoroughly.
 *
 * https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6glyf.html
 */
Glyph FontReader::getCompoundGlyph() {
  std::vector<GlyphComponent> components;
  uint16_t flags;
  BoundingRect rect;
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

    PRINT_VAR(glyphCode);
    PRINT_VAR(isXyValue);
    PRINT_VAR(roundXyGrid);
    PRINT_VAR(hasScale);
    PRINT_VAR(hasXScale);
    PRINT_VAR(hasTwoByTwo);
    PRINT_VAR(hasInstruction);
    PRINT_VAR(useMetrics);
    PRINT_VAR(overlap);
    std::wcout << std::endl;

    // read arguments (arg1,arg2) either words or bytes (signed)
    int32_t arg1 = 0, arg2 = 0;
    if (isWord) {
      arg1 = readInt16();
      arg2 = readInt16();
    } else {
      arg1 = static_cast<int32_t>(static_cast<unsigned>(readInt8()));
      arg2 = static_cast<int32_t>(static_cast<unsigned>(readInt8()));
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
    const auto currentBytePos = ifs.tellg();
    const auto glyph =
        getGlyphByOffset(glyphCodeToOffset[glyphCode], affineMat);
    const auto lc = glyph.getComponents();
    if (lc.empty()) {
      throw std::runtime_error(
          "FontReader: Something went wrong with loading component glyphs");
    }
    components.insert(components.end(), lc.begin(), lc.end());

    if (useMetrics) {
      // TODO: Maybe "metrics" does not mean bounding rect, but positioning??
      rect = lc[0].getBoundingRect();
    }
    // Reload the prev reader pos for the next loop
    goTo(currentBytePos);
  } while (Bit::isFlagSet(flags, 5)); // MORE_COMPONENTS

  return Glyph(components, rect);
}

uint32_t FontReader::getGlyphOffsetByUnicode(const wchar_t unicode) {
  // Convert Unicode -> glyph code (glyph index) -> glyph offset
  if (unicodeToGlyphCode.contains(unicode)) {
    const auto glyphCode = unicodeToGlyphCode[unicode];
    return glyphCodeToOffset[glyphCode];
  }
  throw std::runtime_error(
      "FontReader: Does not have such glyph");
}

std::vector<int> FontReader::readGlyphCoordinates(const uint16_t& n,
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