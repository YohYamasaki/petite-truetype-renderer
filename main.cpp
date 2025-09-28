#include <fstream>
#include <iostream>

#include "FontReader.h"

int main() {
  FontReader reader("fonts/Roboto-Regular.ttf");

  reader.skipBytes(sizeof(uint32_t));  // skip sfntVersion
  const uint16_t numTables = reader.readUint16();

  std::cout << "numTables: " << numTables << std::endl;
  reader.skipBytes(sizeof(uint16_t) *
                   3);  // skip searchRange, entrySelector, rangeShift

  for (int i = 0; i < numTables; ++i) {
    const auto tag = reader.readTag();
    const auto checkSum = reader.readUint32();
    const auto offset = reader.readUint32();
    const auto length = reader.readUint32();
    std::cout << "tag: " << tag << " "
              << "offset: " << offset << std::endl;
  }
  return 0;
}
