//
// Created by yoh on 03/10/25.
//

#ifndef DEBUG_H
#define DEBUG_H
#include <iostream>

#define PRINT_VAR(x) std::wcout << std::dec << #x L" = " << (x) << std::endl
#define PRINT_VAR_HEX(x) std::wcout << std::hex << #x L" = " << (x) << std::dec << std::endl
#define PRINT_VAR_WCHAR(x) std::wcout << #x L" = " << (wchar_t)(x) << std::endl

#endif //DEBUG_H
