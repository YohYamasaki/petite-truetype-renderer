//
// Created by yoh on 28/09/25.
//

#ifndef FONTREADER_H
#define FONTREADER_H
#include <fstream>
#include <map>

#include "Glyph.h"


#endif  // FONTREADER_H

struct Tag {
  uint32_t checkSum;
  uint32_t offset;
  uint32_t length;
};

class FontReader {
public:
  explicit FontReader(const std::string& path);

  void skipBytes(unsigned int bytes);
  void goTo(unsigned int targetByte);
  uint8_t readUint8();
  uint16_t readUint16();
  uint32_t readUint32();
  int8_t readInt8();
  int16_t readInt16();
  int32_t readInt32();

  std::string readTag();
  void loadGlyphOffsets();
  uint32_t getGlyphOffsetByUnicode(wchar_t unicode);
  void loadUnicodeToGlyphCodeTable();
  void readCmapFormat12();
  Glyph getGlyph(const std::string& s);

private:
  std::ifstream ifs;
  std::map<std::string, Tag> directory;
  std::map<uint32_t, uint32_t> unicodeToGlyphCode;
  std::map<uint32_t, uint32_t> glyphCodeToOffset;
  std::vector<int> readGlyphCoordinates(const uint16_t& n,
                                   const std::vector<uint8_t>& flags, bool isX);

  template <class T>
  T readBeOrThrow();
};