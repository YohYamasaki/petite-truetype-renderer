//
// Created by yoh on 03/10/25.
//

#pragma once
#ifndef DEBUG_H
#define DEBUG_H
#include <iostream>
#include <glm/glm.hpp>

#define PRINT_VAR(x) std::wcout << std::dec << #x L" = " << (x) << std::endl
#define PRINT_VAR_HEX(x) std::wcout << std::hex << #x L" = " << (x) << std::dec << std::endl
#define PRINT_VAR_WCHAR(x) std::wcout << #x L" = " << (wchar_t)(x) << std::endl

inline void printMat3(const glm::mat3& m) {
  for (int row = 0; row < 3; ++row) {
    std::wcout << "[ ";
    for (int col = 0; col < 3; ++col) {
      std::wcout << m[col][row];
      if (col < 2) std::wcout << ", ";
    }
    std::wcout << " ]\n";
  }
}

#endif //DEBUG_H
