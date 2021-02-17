//
// Created by admin on 2021/2/5.
//

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <cstdint>
#include "net/ws_cli/ws_types.h"
namespace ws {
const char* Opcode2Str(const uint8_t opcode);

uint64_t EncodeN2H64(const uint64_t input);
uint64_t EncodeH2N64(const uint64_t input);
int GetRandomMask(unsigned char *buf, size_t len);
} // namespace ws

#endif //TEST_UTIL_H
