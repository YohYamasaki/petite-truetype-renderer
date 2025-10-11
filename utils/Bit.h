#pragma once
#ifndef UTILS_H
#define UTILS_H

inline bool isFlagSet(const int flag, const int targetIdx) {
  return ((flag >> targetIdx) & 1) == 1;
}

#endif  // UTILS_H
