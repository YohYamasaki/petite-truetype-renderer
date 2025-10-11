#pragma once
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <utility>

inline std::pair<uint32_t, int> decodeUtf8Char(const char* s,
                                               const int remaining) {
  if (remaining <= 0) throw std::runtime_error("Empty input");
  unsigned char c0 = static_cast<unsigned char>(s[0]);

  // 1-byte (ASCII)
  if (c0 < 0x80) return {c0, 1};

  // 2-byte
  if ((c0 & 0xE0) == 0xC0) {
    if (remaining < 2) throw std::runtime_error("Truncated UTF-8 (2-byte)");
    const unsigned char c1 = static_cast<unsigned char>(s[1]);
    if ((c1 & 0xC0) != 0x80)
      throw std::runtime_error(
          "Invalid UTF-8 continuation byte");
    uint32_t cp = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
    return {cp, 2};
  }

  // 3-byte
  if ((c0 & 0xF0) == 0xE0) {
    if (remaining < 3) throw std::runtime_error("Truncated UTF-8 (3-byte)");
    const unsigned char c1 = static_cast<unsigned char>(s[1]);
    const unsigned char c2 = static_cast<unsigned char>(s[2]);
    if (((c1 & 0xC0) != 0x80) || ((c2 & 0xC0) != 0x80))
      throw std::runtime_error("Invalid UTF-8 continuation byte");
    uint32_t cp = ((c0 & 0x0F) << 12)
                  | ((c1 & 0x3F) << 6)
                  | (c2 & 0x3F);
    return {cp, 3};
  }

  // 4-byte
  if ((c0 & 0xF8) == 0xF0) {
    if (remaining < 4) throw std::runtime_error("Truncated UTF-8 (4-byte)");
    const unsigned char c1 = static_cast<unsigned char>(s[1]);
    const unsigned char c2 = static_cast<unsigned char>(s[2]);
    const unsigned char c3 = static_cast<unsigned char>(s[3]);
    if (((c1 & 0xC0) != 0x80) || ((c2 & 0xC0) != 0x80) || ((c3 & 0xC0) != 0x80))
      throw std::runtime_error("Invalid UTF-8 continuation byte");
    uint32_t cp = ((c0 & 0x07) << 18)
                  | ((c1 & 0x3F) << 12)
                  | ((c2 & 0x3F) << 6)
                  | (c3 & 0x3F);
    return {cp, 4};
  }
  throw std::runtime_error("Invalid UTF-8");
}

inline std::vector<uint32_t> utf8ToCodepoints(const std::string& s) {
  std::vector<uint32_t> out;
  const char* data = s.data();
  const int len = static_cast<int>(s.size());
  int idx = 0;
  while (idx < len) {
    auto [cp, nbytes] = decodeUtf8Char(data + idx, len - idx);
    out.emplace_back(cp);
    idx += nbytes;
  }
  return out;
}
