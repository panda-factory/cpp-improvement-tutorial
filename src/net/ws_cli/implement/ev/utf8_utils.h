//
// Created by admin on 2021/2/11.
//

#ifndef TEST_UTF8_UTILS_H
#define TEST_UTF8_UTILS_H
#include <cstdint>
namespace ws {

#define WS_UTF8_ACCEPT 0
#define WS_UTF8_REJECT 1
#define WS_UTF8_NEEDMORE 2
using Utf8State = uint32_t;

Utf8State DecodeUtf8(Utf8State *state, uint32_t *codep, uint32_t byte);

Utf8State ValidateUtf8(Utf8State *state, const char *str, size_t len);
} // namespace ws


#endif //TEST_UTF8_UTILS_H
