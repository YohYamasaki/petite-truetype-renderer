//
// Created by yoh on 28/09/25.
//

#ifndef FONTREADER_H
#define FONTREADER_H
#include <fstream>

#endif  // FONTREADER_H

class FontReader {
 public:
  explicit FontReader(const std::string& path);
  void skipBytes(unsigned int bytes);
  uint16_t readUint16();
  uint32_t readUint32();
  std::string readTag();

 private:
  std::ifstream ifs;
};