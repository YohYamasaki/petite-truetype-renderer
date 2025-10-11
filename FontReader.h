#pragma once
#ifndef FONTREADER_H
#define FONTREADER_H
#include <fstream>
#include <map>
#include <unordered_map>
#include <glm/glm.hpp>

#include "Glyph.h"


#endif  // FONTREADER_H

struct Tag {
  uint32_t checkSum;
  uint32_t offset;
  uint32_t length;
};

struct GlyphHeader {
  int16_t numOfContours;
  BoundingRect rect;
};

struct FontMetric {
  int16_t ascent;
  int16_t descent;
};

class FontReader {
public:
  explicit FontReader(const std::string& path);
  /**
   * Get general metrics for font.
   * @return FontMetric that has ascent and descent of font
   */
  FontMetric getFontMetric();
  /**
   * Get glyphs and required rendering width from Unicode codepoints.
   * @param cps Vector of Unicode codepoints
   * @param scale Scaling value of glyph size
   * @return Pair of Glyphs and necessary width for rendering
   */
  std::pair<std::vector<Glyph>, int> getGlyphs(std::vector<uint32_t> cps,
                                               float scale);
  /**
   * Get glyph by Unicode.
   * @param cp Unicode codepoint
   * @return Glyph
   */
  Glyph getGlyph(uint32_t cp);

private:
  std::ifstream ifs;
  std::map<std::string, Tag> directory;
  std::unordered_map<uint32_t, uint16_t> unicodeToGlyphCode;
  std::unordered_map<uint16_t, uint32_t> glyphCodeToOffset;
  std::unordered_map<uint16_t, Metric> glyphMetric;

  // stream helper methods
  void skipBytes(unsigned bytes);
  void jumpTo(unsigned byteOffset);
  /**
   * Read data from stream.
   * @tparam T Target type
   * @return Read value
   */
  template <class T>
  T readBeOrThrow();
  uint8_t readUint8();
  uint16_t readUint16();
  uint32_t readUint32();
  int8_t readInt8();
  int16_t readInt16();
  int32_t readInt32();
  float readF2Dot14();

  // Initializer methods
  /**
   * Load the glyph code to offset mapping from `loca` table and
   * store them in the glyphCodeToOffset hashmap.
   */
  void loadGlyphOffsetsMap();
  /**
   * Load the glyph code to metrics from `hmtx` table and
   * store them in the  glyphMetric hashmap.
   */
  void loadGlyphMetricsMap();
  /**
   * Load Unicode to Glyph code table. Glyph code is not the offset in ttf file,
   * the glyphCodeToOffset is there to retrieve it.
   * Only supports format 12 table for now.
   * https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
   */
  void loadUnicodeToGlyphCodeMap();

  // Glyph related methods
  /**
   * Read glyph header data.
   * @param glyphCode Glyph code
   * @return GlyphHeader data
   */
  GlyphHeader readGlyphHeader(uint16_t glyphCode);
  /**
   * Get glyph by glyph code.
   *
   * @param glyphCode Glyph code
   * @return Glyph
   */
  Glyph getGlyphByCode(uint16_t glyphCode);
  /**
   * Get glyph components for the compound glyph.
   * A compound glyph can have multiple components,
   * which can be either simple glyph structure or another compound glyph.
   * @param glyphCode Glyph code
   * @param affineMat 3x3 matrix for affine transformation
   * @return Vector of the glyph components
   */
  std::vector<GlyphComponent> getCompoundSubComponents(
      uint16_t glyphCode,
      const glm::mat3& affineMat);
  /**
   * Get a single glyph component.
   * @param numOfContours number of contours of the target component
   * @param boundingRect bounding rectangle of the target component
   * @param affineMat 3x3 matrix for affine transformation
   * @return Glyph component
   */
  GlyphComponent getGlyphComponent(
      short numOfContours,
      ::BoundingRect boundingRect,
      const glm::mat3& affineMat = glm::mat3(
          1.0f));
  /**
   * Get coordinates for a glyph.
   * @param n Number of vertices
   * @param flags Flags
   * @param isX is for X coordinate
   * @return vector of coordinates
   */
  std::vector<int> getGlyphCoordinates(const uint16_t& n,
                                       const std::vector<uint8_t>& flags,
                                       bool isX);
  /**
   * Get compound glyph.
   * ARGS_ARE_XY_VALUES, ROUND_XY_TO_GRID, WE_HAVE_INSTRUCTIONS, OVERLAP_COMPOUND
   * are not supported.
   * @return Glyph
   */
  Glyph getCompoundGlyph();
};
