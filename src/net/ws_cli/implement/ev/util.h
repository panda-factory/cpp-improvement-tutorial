//
// Created by admin on 2021/2/5.
//

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <cstdint>
#include "net/ws_cli/ws_types.h"
namespace ws {
const char* Opcode2Str(const uint8_t opcode);
// Converts binarDy data of length=len to base64 characters.
// Length of the resultant string is stored in flen
// (you must pass pointer flen).
char* Encode64(const void* binaryData, int len, int *flen);

unsigned char* Decode64( const char* ascii, int len, int *flen );

uint64_t EncodeN2H64(const uint64_t input);
uint64_t EncodeH2N64(const uint64_t input);

} // namespace ws

#endif //TEST_UTIL_H
