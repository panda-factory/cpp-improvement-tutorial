//
// Created by admin on 2021/2/9.
//

#ifndef TEST_WSHEADER_H
#define TEST_WSHEADER_H

#include <memory>
#include "net/ws_cli/ws_types.h"

namespace ws {
void PackHeader(WSHeader& h, uint8_t *b, size_t len, size_t *header_len);
}// namespace ws

#endif //TEST_WSHEADER_H
