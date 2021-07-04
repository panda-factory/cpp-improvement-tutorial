//
// Created by admin on 2021/2/5.
//

#ifndef TEST_WS_STATE_H
#define TEST_WS_STATE_H
namespace net {
namespace ws {
enum class WSSendState {
    NONE,
    MESSAGE_BEGIN,
    IN_MESSAGE,
    IN_MESSAGE_PAYLOAD
};
enum class WSConnectState {
    ERROR = -1,
    NONE = 0,
    SENT_REQ,
    PARSED_STATUS,
    PARSED_HEADERS,
    HANDSHAKE_COMPLETE
};
enum class WSParseState {
    USER_ABORT = -2,
    ERROR = -1,
    SUCCESS = 0,
    NEED_MORE
};
constexpr uint8_t WS_HAS_VALID_UPGRADE_HEADER 	    = (1 << 0); ///< A valid Upgrade header received.
constexpr uint8_t WS_HAS_VALID_CONNECTION_HEADER 	= (1 << 1); ///< A valid Connection header received.
constexpr uint8_t WS_HAS_VALID_WS_ACCEPT_HEADER 	= (1 << 2); ///< A valid Sec-WebSocket-Accept header received.
constexpr uint8_t WS_HAS_VALID_WS_EXT_HEADER 		= (1 << 3); ///< A valid Sec-WebSocket-Extensions header received.
constexpr uint8_t WS_HAS_VALID_WS_PROTOCOL_HEADER   = (1 << 4); ///< A valid Sec-WebSocket-Protocol header received.

enum class WSState {
    DNS_LOOKUP,
    CLOSING,
    CLOSING_UNCLEANLY,
    CLOSED_CLEANLY,
    CLOSED_UNCLEANLY,
    CONNECTING,
    CONNECTED
};
enum class WSCloseStatus {
    /// 1000 indicates a normal TaskType, meaning that the purpose for
    /// which the connection was established has been fulfilled.
    NORMAL_1000 = 1000,

    /// 1001 indicates that an endpoint is "going away", such as a server
    /// going down or a browser having navigated away from a page.
    GOING_AWAY_1001 = 1001,

    /// 1002 indicates that an endpoint is terminating the connection due
    /// to a protocol error.
    PROTOCOL_ERR_1002 = 1002,

    /// 1003 indicates that an endpoint is terminating the connection
    /// because it has received a type of data it cannot accept (e.g., an
    /// endpoint that understands only text data MAY send this if it
    /// receives a binary message).
    UNSUPPORTED_DATA_1003 = 1003,

    /// Reserved.  The specific meaning might be defined in the future.
    RESERVED_1004 = 1004,

    /// 1005 is a reserved value and MUST NOT be set as a status code in a
    /// Close control frame by an endpoint.  It is designated for use in
    /// applications expecting a status code to indicate that no status
    /// code was actually present.
    STATUS_CODE_EXPECTED_1005 = 1005,

    /// 1006 is a reserved value and MUST NOT be set as a status code in a
    /// Close control frame by an endpoint.  It is designated for use in
    /// applications expecting a status code to indicate that the
    /// connection was closed abnormally, e.g., without sending or
    /// receiving a Close control frame.
    ABNORMAL_1006 = 1006,

    /// 1007 indicates that an endpoint is terminating the connection
    /// because it has received data within a message that was not
    /// consistent with the type of the message (e.g., non-UTF-8 [RFC3629]
    /// data within a text message).
    INCONSISTENT_DATA_1007 = 1007,

    /// 1008 indicates that an endpoint is terminating the connection
    /// because it has received a message that violates its policy.  This
    /// is a generic status code that can be returned when there is no
    /// other more suitable status code (e.g., 1003 or 1009) or if there
    /// is a need to hide specific details about the policy.
    POLICY_VIOLATION_1008 = 1008,

    /// 1009 indicates that an endpoint is terminating the connection
    /// because it has received a message that is too big for it to
    /// process.
    MESSAGE_TOO_BIG_1009 = 1009,

    /// 1010 indicates that an endpoint (client) is terminating the
    /// connection because it has expected the server to negotiate one or
    /// more extension, but the server didn't return them in the response
    /// message of the WebSocket handshake.  The list of extensions that
    /// are needed SHOULD appear in the /reason/ part of the Close frame.
    /// Note that this status code is not used by the server, because it
    /// can fail the WebSocket handshake instead.
    EXTENSION_NOT_NEGOTIATED_1010 = 1010,

    /// 1011 indicates that a server is terminating the connection because
    /// it encountered an unexpected condition that prevented it from
    /// fulfilling the request.
    UNEXPECTED_CONDITION_1011 = 1011,

    /// 1015 is a reserved value and MUST NOT be set as a status code in a
    /// Close control frame by an endpoint.  It is designated for use in
    /// applications expecting a status code to indicate that the
    /// connection was closed due to a failure to perform a TLS handshake
    /// (e.g., the server certificate can't be verified).
    FAILED_TLS_HANDSHAKE_1015 = 1015
};
} // namespace ws
} // namespace net
#endif //TEST_WS_STATE_H
