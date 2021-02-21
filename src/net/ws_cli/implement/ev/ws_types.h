//
// Created by admin on 2021/2/6.
//

#ifndef TEST_WS_TYPES_H
#define TEST_WS_TYPES_H

#include <functional>

#include "ws_state.h"

namespace ws {
constexpr int WS_DEFAULT_CONNECT_TIMEOUT = 60;
constexpr int HTTP_STATUS_SWITCHING_PROTOCOLS_101 = 101;
constexpr int  WS_HDR_BASE_SIZE = 2;
constexpr int  WS_HDR_PAYLOAD_LEN_SIZE = 8;
constexpr int  WS_HDR_MASK_SIZE = 4;
constexpr int  WS_HDR_MIN_SIZE = WS_HDR_BASE_SIZE;
constexpr int  WS_HDR_MAX_SIZE = (WS_HDR_BASE_SIZE + WS_HDR_PAYLOAD_LEN_SIZE + WS_HDR_MASK_SIZE);

enum WSOpcode {
    CONTINUATION_0X0 = 0x0,
    TEXT_0X1 = 0x1,
    BINARY_0X2 = 0x2,

    // 0x3 - 0x7 Reserved for further non-control frames.
    NON_CONTROL_RSV_0X3 = 0x3,
    NON_CONTROL_RSV_0X4 = 0x4,
    NON_CONTROL_RSV_0X5 = 0x5,
    NON_CONTROL_RSV_0X6 = 0x6,
    NON_CONTROL_RSV_0X7 = 0x7,

    CLOSE_0X8 = 0x8,
    PING_0X9 = 0x9,
    PONG_0XA = 0xA,

    // 0xB - 0xF are reserved for further control frames.
    CONTROL_RSV_0XB = 0xB,
    CONTROL_RSV_0XC = 0xC,
    CONTROL_RSV_0XD = 0xD,
    CONTROL_RSV_0XE = 0xE,
    CONTROL_RSV_0XF = 0xF
};

///
/// Websocket header structure.
///
///
/// Websocket protocol RFC 6455: http://tools.ietf.org/html/rfc6455
///
///      0                   1                   2                   3
///      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
///     +-+-+-+-+-------+-+-------------+-------------------------------+
///     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
///     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
///     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
///     | |1|2|3|       |K|             |                               |
///     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
///     |     Extended payload length continued, if payload len == 127  |
///     + - - - - - - - - - - - - - - - +-------------------------------+
///     |                               |Masking-key, if MASK set to 1  |
///     +-------------------------------+-------------------------------+
///     | Masking-key (continued)       |          Payload Data         |
///     +-------------------------------- - - - - - - - - - - - - - - - +
///     :                     Payload Data continued ...                :
///     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
///     |                     Payload Data continued ...                |
///     +---------------------------------------------------------------+
///
#pragma pack(push,1)
struct WSHeader {
    uint8_t fin : 1;            //! Indicates that this is the final fragment in a message.
    uint8_t rsv1 : 1;            ///< Reserved bit 1 for extensions.
    uint8_t rsv2 : 1;            ///< Reserved bit 2 for extensions.
    uint8_t rsv3 : 1;            ///< Reserved bit 3 for extensions.
    uint8_t opcode : 4;        ///< Operation code.
    uint8_t maskBit : 1;        ///< If this frame is masked this bit is set.
    uint8_t payloadLen : 7;    ///< Length of the payload.
    uint64_t exPayloadLen;	    ///< Length of the payload.
    uint32_t mask;            ///< Masking key for the payload.
};
#pragma pack(pop)

#define WS_IS_CLOSE_STATUS_NOT_USED(code) \
	(((int)code < 1000) || ((int)code > 4999))

#define WS_MAX_PAYLOAD_LEN 0x7FFFFFFFFFFFFFFF
#define WS_CONTROL_MAX_PAYLOAD_LEN 125

/// Status codes in the range 3000-3999 are reserved for use by
/// libraries, frameworks, and applications.  These status codes are
/// registered directly with IANA.  The interpretation of these codes
/// is undefined by this protocol.
#define WS_IS_CLOSE_STATUS_IANA_RESERVED(code) \
	(((int)code >= 3000) && ((int)code <= 3999))

/// Status codes in the range 4000-4999 are reserved for private use
/// and thus can't be registered.  Such codes can be used by prior
/// agreements between WebSocket applications.  The interpretation of
/// these codes is undefined by this protocol.
#define WS_IS_CLOSE_STATUS_PRIVATE_USE(code) \
	(((int)code >= 4000) && ((int)code <= 4999))

/// Has the peer sent a valid close code?
#define WS_IS_PEER_CLOSE_STATUS_VALID(code) \
	(WS_IS_CLOSE_STATUS_IANA_RESERVED(code) \
	|| (WS_IS_CLOSE_STATUS_PRIVATE_USE(code)) \
	|| (code == WSCloseStatus::NORMAL_1000) \
	|| (code == WSCloseStatus::GOING_AWAY_1001) \
	|| (code == WSCloseStatus::PROTOCOL_ERR_1002) \
	|| (code == WSCloseStatus::UNSUPPORTED_DATA_1003) \
	|| (code == WSCloseStatus::INCONSISTENT_DATA_1007) \
	|| (code == WSCloseStatus::POLICY_VIOLATION_1008) \
	|| (code == WSCloseStatus::MESSAGE_TOO_BIG_1009) \
	|| (code == WSCloseStatus::EXTENSION_NOT_NEGOTIATED_1010) \
	|| (code == WSCloseStatus::UNEXPECTED_CONDITION_1011))

using NoCopyCleanup = void (*)(const void *data, uint64_t datalen, void *extra);
using CloseCallback = void (*)(WSCloseStatus status, const char *reason, size_t reason_len, void *arg);
using MsgBeginCallback = void (*)(void *arg);
using MsgEndCallback =  void (*)(void *arg);
using MsgFrameBeginCallback = void (*)(void *arg);
using MsgFrameDataCallback = void (*)(char *payload,
                                             uint64_t len, void *arg);
using MsgCallback = void (*)(char *msg, uint64_t len,
                                  int binary, void *arg);
using MsgFrameEndCallback = void (*)(void *arg);
using ConnectHandle = std::function<void(void *arg)>;
} // namespace ws
#endif //TEST_WS_TYPES_H
