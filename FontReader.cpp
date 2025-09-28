//
// Created by yoh on 28/09/25.
//

#include "FontReader.h"

#include <iostream>

FontReader::FontReader(const std::string& path) {
  ifs.open("fonts/Roboto-Regular.ttf", std::ios::binary);
  if (!ifs) {
    std::cerr << "failed to open file\n";
  }
}

void FontReader::skipBytes(const unsigned int bytes) {
  ifs.seekg(bytes, std::ios::cur);
  std::cout << "skipped to: " << ifs.tellg() << std::endl;
}

uint16_t FontReader::readUint16() {
  constexpr unsigned int bytesLength = 2;
  char b[bytesLength - 1];
  ifs.read(b, bytesLength);
  return static_cast<uint16_t>(b[0] << 8) | (static_cast<uint16_t>(b[1]));
};

uint32_t FontReader::readUint32() {
  constexpr unsigned int bytesLength = 4;
  char b[bytesLength - 1];
  ifs.read(b, bytesLength);
  return static_cast<uint16_t>(b[0] << 24) |
         (static_cast<uint16_t>(b[1]) << 16) |
         static_cast<uint16_t>(b[2] << 8) | (static_cast<uint16_t>(b[3]));
};

std::string FontReader::readTag() {
  constexpr unsigned int bytesLength = 4;
  char b[bytesLength + 1];
  ifs.read(b, bytesLength);
  b[bytesLength] = '\0';
  std::string str(b);
  return str;
}