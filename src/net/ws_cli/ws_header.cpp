//
// Created by admin on 2021/2/9.
//

#include "ws_header.h"
#include "core/logging.h"
#include "util.h"
#ifndef _WIN32
#include <arpa/inet.h>
#endif

#ifdef _WIN32
#include <winsock.h>
#endif

namespace net {
namespace ws {
namespace {
void PackHeaderFirstByte(WSHeader *h, uint8_t *b) {
    WTF_CHECK(h);
    WTF_CHECK(b);

    //  7 6 5 4 3 2 1 0
    // +-+-+-+-+-------+
    // |F|R|R|R| opcode|
    // |I|S|S|S|  (4)  |
    // |N|V|V|V|       |
    // | |1|2|3|       |
    // +-+-+-+-+-------+
    b[0] = 0;
    b[0] |= ((!!h->fin)  << 7);
    b[0] |= ((!!h->rsv1) << 6);
    b[0] |= ((!!h->rsv2) << 5);
    b[0] |= ((!!h->rsv3) << 4);
    b[0] |= (h->opcode  & 0xF);
}

void PackHeaderRest(WSHeader *h, uint8_t *b, size_t len, size_t *header_len) {
    WTF_CHECK(h);
    WTF_CHECK(b);
    WTF_CHECK(len >= WS_HDR_MAX_SIZE);
    WTF_CHECK(h->payloadLen <= WS_MAX_PAYLOAD_LEN);
    WTF_CHECK(header_len);

    // +----------------+---------------+-------------------------------+
    // |    Byte 0      |     Byte 1    |     Byte 2+                   |
    // +----------------+---------------+-------------------------------+
    //					 7 6 5 4 3 2 1 0
    //                  +-+-------------+-------------------------------+
    //                  |M| Payload len |    Extended payload length    |
    //                  |A|     (7)     |             (16/64)           |
    //                  |S|             |   (if payload len==126/127)   |
    //                  |K|             |                               |
    //  +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
    //  |     Extended payload length continued, if payload len == 127  |
    //  + - - - - - - - - - - - - - - - +-------------------------------+
    //  |                               |Masking-key, if MASK set to 1  |
    //  +-------------------------------+-------------------------------+
    //  | Masking-key (continued)       |
    //  +--------------------------------

    b[1] = 0;

    // Masking bit.
    // (This MUST be set for a client according to RFC).
    b[1] |= ((!!h->maskBit) << 7);

    *header_len = 2;

    if (h->payloadLen <= 125)
    {
        // Use 1 byte for payload len.
        b[1] |= h->payloadLen;
    }
    else if (h->payloadLen <= 0xFFFF)
    {
        // 2 byte extra payload length.
        uint16_t *size_ptr = (uint16_t *)&b[2];
        *header_len += 2;

        b[1] |= 126;
        *size_ptr = htons((uint16_t)h->payloadLen);
    }
    else
    {
        // 8 byte extra payload length.
        uint64_t *size_ptr = (uint64_t *)&b[2];
        *header_len += 8;

        b[1] |= 127;
        *size_ptr = EncodeH2N64(h->payloadLen);
    }

    if (h->maskBit)
    {
        uint32_t *mask_ptr = (uint32_t *)&b[*header_len];
        *mask_ptr = (h->mask);
        *header_len += 4;
    }
}
} // namespace

void PackHeader(WSHeader& h, uint8_t *b, size_t len, size_t *header_len) {
    PackHeaderFirstByte(&h, b);
    PackHeaderRest(&h, b, len, header_len);
}

} // namespace ws
} // namespace net