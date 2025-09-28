//
// Created by yoh on 28/09/25.
//

#include "FontReader.h"

#include <iostream>

#include "utils/Bit.h"
#include "utils/Debug.h"
#include "utils/Geometry.h"

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
 * Only supports format 12 table for now
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

Glyph FontReader::getGlyph(const wchar_t unicode) {
  goTo(getGlyphOffsetByUnicode(unicode));
  // Read glyph description
  const auto numOfContours = readInt16(); // number of contours
  // TODO
  if (numOfContours < 0) {
    throw std::runtime_error(
        "FontReader: Currently ignoring compound glyphs");
  };

  BoundingRect boundingRect;
  boundingRect.xMin = readInt16();
  boundingRect.yMin = readInt16();
  boundingRect.xMax = readInt16();
  boundingRect.yMax = readInt16();

  // Read simple glyphs
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

  Glyph glyph{numOfVertices, endPtsOfContours, boundingRect, xCoordinates,
              yCoordinates};
  return glyph;
}

uint32_t FontReader::getGlyphOffsetByUnicode(const wchar_t unicode) {
  // Convert Unicode -> glyph code -> glyph offset
  if (unicodeToGlyphCode.contains(unicode)) {
    const auto glyphCode = unicodeToGlyphCode[unicode];
    return glyphCodeToOffset[glyphCode];
  }

  std::cout << "Not supported yet!" << std::endl;
  return 0;
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
      const auto offset = isPositive ? readUint8() : -1 * readUint8();
      pos += offset;
    } else {
      if (!Bit::isFlagSet(f, isX ? 4 : 5)) pos += readInt16();
    }

    coordinates[i] = pos;
  }
  return coordinates;
}



