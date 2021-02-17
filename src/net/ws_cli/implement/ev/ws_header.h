//
// Created by admin on 2021/2/9.
//

#ifndef TEST_WSHEADER_H
#define TEST_WSHEADER_H

#include <memory>
#include "net/ws_cli/ws_types.h"

namespace ws {

inline bool IsControlOpcode(const uint8_t code) {
    const WSOpcode opcode = static_cast<WSOpcode>(code);
    return (opcode >= WSOpcode::CLOSE_0X8) &&
           (opcode <= WSOpcode::CONTROL_RSV_0XF);
}

inline bool IsReservedControlOpcode(const uint8_t code) {
    const WSOpcode opcode = static_cast<WSOpcode>(code);
    return (opcode >= WSOpcode::CONTROL_RSV_0XB) &&
           (opcode <= WSOpcode::CONTROL_RSV_0XF);
}

inline bool IsReservedNonControlOpcode(const uint8_t code) {
    const WSOpcode opcode = static_cast<WSOpcode>(code);
    return (opcode >= WSOpcode::NON_CONTROL_RSV_0X3) &&
           (opcode <= WSOpcode::NON_CONTROL_RSV_0X7);
}

inline bool IsReservedOpcode(const uint8_t code) {
    return IsReservedControlOpcode(code) ||
            IsReservedNonControlOpcode(code);
}
void PackHeader(WSHeader& h, uint8_t *b, size_t len, size_t *header_len);
}// namespace ws

#endif //TEST_WSHEADER_H
