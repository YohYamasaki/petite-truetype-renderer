//
// Created by yoh on 28/09/25.
//

#include "Bit.h"

bool Bit::isFlagSet(const int flag, const int targetIdx) {
  return ((flag >> targetIdx) & 1) == 1;
}