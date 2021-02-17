//
// Created by admin on 2021/2/4.
//

#ifdef WIN32
#define _CRT_RAND_S
#include <stdlib.h>
#endif

#include "net/ws_cli/implement/ev/session_impl_ev.h"

#include <openssl/sha.h>

#include "net/ws_cli/implement/ev/util.h"
#include "core/logging.h"
#include "core/fmt_string.h"
#include "core/scope_exit.h"
#include "ws_header.h"
#include "base64.h"

#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#undef ERROR

namespace ws {
namespace {
bool CompareStrIgnoreCase(const std::string& a, const std::string& b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      [](char a, char b) {
                          return tolower(a) == tolower(b);
                      });
}

void MaskPayload(uint32_t mask, char *msg, uint64_t len)
{
    uint8_t *m = (uint8_t *)&mask;
    uint8_t *p = (uint8_t *)msg;

    if (!msg || !len)
        return;

    for (size_t i = 0; i < len; i++) {
        p[i] ^= m[i % 4];
    }
}

void UnmaskPayload(uint32_t mask, char *msg, uint64_t len)
{
    MaskPayload(mask, msg, len);
}

} // namespace

// | static |
void SessionImplEV::BuiltinNoCopyCleanupWrapper(const void *data, size_t dataLen, void *extra) {
    SessionImplEV* thiz = (SessionImplEV*)(extra);
    WTF_CHECK(thiz);
    WTF_CHECK(thiz->noCopyCleanupCb_);

    // We wrap this so we can pass the websocket context.
    // (Also, we don't want to expose any bufferevent types to the
    //  external API so we're free to replace it).
    thiz->noCopyCleanupCb_(data, dataLen, thiz->noCopyExtra_);
}
// | static |
void SessionImplEV::OnCloseTimeout(evutil_socket_t fd, short what, void *arg) {
    SessionImplEV* thiz = (SessionImplEV*)(arg);
    WTF_CHECK(thiz);

    WTF_LOG(INFO) << "Close timeout";

    // This callback should only ever be called after sending a close frame.
    WTF_CHECK(thiz->sentCloseFlag_);

    // We sent a close frame to the server but it hasn't initiated
    // the TCP close.
    if (thiz->receivedCloseFlag_) {
        WTF_LOG(ERROR) << "Timeout! Server sent a Websocket close frame "
                             "but did not close the TCP session";
    } else {
        WTF_LOG(ERROR) << "Timeout! Server did not reply to Websocket "
                             "close frame";
    }

    WTF_LOG(ERROR) << "Initiating an unclean close";

    thiz->ShutDown();
}
// | static |
void SessionImplEV::OnConnectionTimeout(evutil_socket_t fd, short what, void *arg) {
    char buf[256];
    SessionImplEV* thiz = (SessionImplEV*)(arg);
    WTF_CHECK(thiz);

    WTF_LOG(ERROR) << wtf::FmtString("Websocket connection timed out after %ld seconds "
                         "for %s", thiz->connectTimeout_.tv_sec,
                                     thiz->GetUri(buf, sizeof(buf)).c_str());

}
// | static |
void SessionImplEV::OnRead(struct bufferevent *bev, void *ObscureData) {
    WTF_LOG(INFO) << "OnRead";

    SessionImplEV* thiz = (SessionImplEV*)(ObscureData);
    WTF_CHECK(thiz);
    WTF_CHECK(bev);
    WTF_CHECK(thiz->bev_ == bev);

    struct evbuffer *inBuf = bufferevent_get_input(thiz->bev_);

    if (thiz->connectState_ != WSConnectState::HANDSHAKE_COMPLETE) {
        // Complete the connection handshake.
        WSParseState state;

        WTF_LOG(INFO) << "Look for handshake reply";

        switch ((state = thiz->ReadServerHandshakeReply(inBuf))) {
            case WSParseState::ERROR:
                // TODO: Do anything else here?
                thiz->ShutDown();
                break;
            case WSParseState::NEED_MORE:
                return;
            case WSParseState::SUCCESS: {
                thiz->state_ = WSState::CONNECTED;

                thiz->onConnect_();
            }
            case WSParseState::USER_ABORT:
                // TODO: What to do here?
                break;
        }
    }

    // Connected and completed handshake we can now expect websocket data.
    thiz->ReadWebsocket(inBuf);
}
// | static |
void SessionImplEV::OnWrite(bufferevent* bev, void* ObscureData) {
    WTF_LOG(INFO) << "OnWrite";
}
// | static |
void SessionImplEV::OnEvent(bufferevent* bev, short events, void* ObscureData) {
    WTF_LOG(INFO) << "OnEvent";

    SessionImplEV* thiz = (SessionImplEV*)(ObscureData);
    WTF_CHECK(thiz);


    if (events & BEV_EVENT_CONNECTED) {
        thiz->OnConnectEvent(bev, events);
        return;
    }

    if (events & BEV_EVENT_EOF) {
        thiz->OnEofEvent(bev, events);
        return;
    }

    if (events & BEV_EVENT_ERROR) {
    //    _ws_error_event(bev, events, ws);
        return;
    }

    if (events & BEV_EVENT_TIMEOUT) {
    //    WTF_LOG(LIBWS_DEBUG, "Bufferevent timeout");
    }

    if (events & BEV_EVENT_WRITING) {
    //    WTF_LOG(LIBWS_DEBUG, "   Writing");
    }

    if (events & BEV_EVENT_READING) {
     //   WTF_LOG(LIBWS_DEBUG, "   Reading");
    }
}

int SessionImplEV::Quit(int let_running_events_complete) {
    return QuitDelay(let_running_events_complete, NULL);
}

int SessionImplEV::QuitDelay(int let_running_events_complete, const struct timeval *delay) {
    int ret;

    WTF_LOG(INFO) << "Websocket base quit";

    if (let_running_events_complete)
    {
        ret = event_base_loopexit(evBase_, delay);
    }
    else
    {
        ret = event_base_loopbreak(evBase_);
    }

    return ret;
}

void SessionImplEV::OnEofEvent(struct bufferevent *bev, short events) {
    WTF_LOG(INFO) << "EOF cb";

    WSCloseStatus status;
    struct evbuffer *in;

    in = bufferevent_get_input(bev_);

    if (evbuffer_get_length(in) > 0) {
        WTF_LOG(DEBUG) << wtf::FmtString("Left %u bytes at EOF", evbuffer_get_length(in));

        ReadWebsocket(in);
    }

    status = serverCloseStatus_;

    WTF_LOG(DEBUG) << wtf::FmtString("Sent close frame %s, received close frame %s",
              sentCloseFlag_ ? "TRUE" : "FALSE",
              receivedCloseFlag_ ? "TRUE" : "FALSE");

    ShutDown();

    if (!receivedCloseFlag_) {
        state_ = WSState::CLOSED_UNCLEANLY;
        status = WSCloseStatus::ABNORMAL_1006;
    }

    if (onClose_) {
        WTF_LOG(DEBUG) << "Call close callback";

        onClose_(status, serverReason_,
                     serverReasonLen_,
                     nullptr);
    } else {
        WTF_LOG(DEBUG) << "No close callback";
    }

    Quit(1);
}

void SessionImplEV::OnConnectEvent(bufferevent *bev, short events) {
    char buf[1024];
    WTF_LOG(INFO) << "Connected to ";

    bufferevent_enable(bev_, EV_READ | EV_WRITE);

    // Add the handshake to the send buffer, this will
    // be sent as soon as we're connected.
    if (SendHandshake(bufferevent_get_output(bev_)))
    {
        WTF_LOG(ERROR) << "Failed to assemble handshake";
        return;
    }
}
int SessionImplEV::DoInit() {
#ifdef _WIN32
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;

        wVersionRequested = MAKEWORD(2, 2);

        err = WSAStartup(wVersionRequested, &wsaData);
        if (err != 0) {
            WTF_LOG(ERROR) << "WSAStartup failed with error: " << err;
            return -1;
        }
    }
#endif // _WIN32
    state_ = WSState::CLOSED_CLEANLY;

    if (!(evBase_ = event_base_new())) {
        WTF_LOG(ERROR) << "Out of memory!";
        return -1;
    }
    if (!(dnsBase_ = evdns_base_new(evBase_, 1))) {
        WTF_LOG(ERROR) << "Out of memory!";
        return -1;
    }
    return 0;
}

const std::string SessionImplEV::GetUri(char *buf, size_t bufsize) {

    if (!buf) {
        WTF_LOG(ERROR) << "buf is NULL";
        return {};
    }

    // TODO: Check return value?
    if (evutil_snprintf(buf, bufsize, "%s://%s:%d/%s",
                        (useSsl_ != true) ? "wss" : "ws",
                        host_.c_str(),
                        port_,
                        uri_.c_str()) < 0)
    {
        WTF_LOG(ERROR) << wtf::FmtString("Buffer of size %lu is too small for uri", bufsize);
        return NULL;
    }

    return buf;
}

int SessionImplEV::SetupTimeoutEvent(struct event **ev, struct timeval *tv) {
    WTF_CHECK(ev);
    WTF_CHECK(tv);

    WTF_LOG(INFO) << "Setting up new timeout event";

    if (*ev) {
        event_free(*ev);
        *ev = NULL;
    }

    if (!(*ev = evtimer_new(evBase_, OnConnectionTimeout, this))) {
        WTF_LOG(ERROR) << "Failed to create timeout event";
        return -1;
    }

    if (evtimer_add(*ev, tv)) {
        WTF_LOG(ERROR) << "Failed to add timeout event";
        event_free(*ev);
        *ev = NULL;
        return -1;
    }

    return 0;
}

int SessionImplEV::SetupConnectionTimeout() {
    struct timeval tv = {WS_DEFAULT_CONNECT_TIMEOUT, 0};

    if (connectTimeout_.tv_sec > 0) {
        tv = connectTimeout_;
    }

    return SetupTimeoutEvent(&connectTimeoutEvent_, &tv);
}

int SessionImplEV::DoConnect(const std::string& server, int port, const std::string& uri) {

    WTF_LOG(DEBUG) << "Connect start";
    if ((state_ != WSState::CLOSED_CLEANLY)
        && (state_ != WSState::CLOSED_UNCLEANLY)) {
        WTF_LOG(ERROR) << "Already connected or connecting";
        return -1;
    }

    if (server.empty()) {
        WTF_LOG(ERROR) << "NULL server given";
        return -1;
    }

    uri_ = uri;
    host_ = server;
    port_ = port;

    receivedCloseFlag_ = false;
    sentCloseFlag_ = false;
    inMsgFlag_ = false;

    std::ostringstream oss;
    oss << "http://" << host_ << ":" << port_;
    origin_ = oss.str();
    bev_ = bufferevent_socket_new(evBase_, -1, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS);
    bufferevent_setcb(bev_, SessionImplEV::OnRead, SessionImplEV::OnWrite, SessionImplEV::OnEvent, this);

    bufferevent_socket_connect_hostname(bev_, dnsBase_, AF_INET, server.c_str(), port);
    state_ = WSState::CONNECTING;

    // Setup a timeout event for the connection attempt.
    if (SetupConnectionTimeout()) {
        WTF_LOG(ERROR) << "Failed to setup connection timeout event";
        return -1;
    }

    event_base_dispatch(evBase_);
    return 0;
}

int SessionImplEV::GenerateHandshakeKey() {
    unsigned char randKey[16];

    WTF_LOG(INFO) << "Generate handshake key";

    // A |Sec-WebSocket-Key| header field with a base64-encoded (see
    // Section 4 of [RFC4648]) value that, when decoded, is 16 bytes in
    // length.
    if (GetRandomMask(randKey, sizeof(randKey)) < 0) {
        WTF_LOG(ERROR) << "Failed to get random byte sequence "
                             "for Websocket upgrade handshake key";
        return -1;
    }

    handshakeKeyBase64_ = EncodeBase64(randKey, sizeof(randKey));

    if (handshakeKeyBase64_.empty()) {
        WTF_LOG(ERROR) << "Failed to base64 encode websocket key";
        return -1;
    }

    return 0;
}

int SessionImplEV::ParseHttpHeader(char *line, std::string& headerKey, std::string& headerVal)
{
    std::istringstream iss(line);

    std::getline(iss, headerKey, ':') >> headerVal;

    WTF_LOG(INFO) << headerKey << ": " <<headerVal;
    header_.insert_or_assign(headerKey, headerVal);
    return 0;
}

//! Format: HTTP/1.1 [status]
int SessionImplEV::ParseHttpStatus(const char *line,
                                   int& httpMajorVersion,
                                   int& httpMinorVersion,
                                   int& statusCode)
{
    const char *s = line;
    int n;

    statusCode = -1;
    httpMajorVersion = -1;
    httpMinorVersion = -1;

    if (!line)
        return -1;

    n = sscanf(line, "HTTP/%d.%d %d",
               &httpMajorVersion, &httpMinorVersion, &statusCode);

    if (n != 3)
        return -1;

    return 0;
}

WSParseState SessionImplEV::ReadHttpHeaders(struct evbuffer *in) {
    char *line = nullptr;
    SCOPE_GUARD {
        if (line) free(line);
    };
    std::string headerKey;
    std::string headerVal;
    WSParseState state = WSParseState::NEED_MORE;
    size_t len;
    WTF_CHECK(in);

    // TODO: Set a max header size to allow.

    while ((line = evbuffer_readln(in, &len, EVBUFFER_EOL_CRLF)) != NULL) {
        SCOPE_GUARD {
            if (line) free(line);
            line = nullptr;
        };
        // Check for end of HTTP response (empty line).
        if (*line == '\0') {
            WTF_LOG(INFO) << "End of HTTP request";
            state = WSParseState::SUCCESS;
            break;
        }

        if (ParseHttpHeader(line, headerKey, headerVal)) {
            WTF_LOG(ERROR) << "Failed to parse HTTP upgrade "
                                 "repsonse line: " << line;

            state = WSParseState::ERROR;
            break;
        }

        // Let the user get the header.
        //if (ws->header_cb) {
        //    WTF_LOG(INFO), "	Call header callback");

        //    if (ws->header_cb(ws, header_name, header_val, ws->header_arg))
        //    {
        //        WTF_LOG(LIBWS_DEBUG, "User header callback cancelled "
        //                               "handshake";
        //        state = WS_PARSE_STATE_USER_ABORT;
        //        break;
        //    }
        //}

        if (ValidateHttpHeaders(headerKey, headerVal)) {
            WTF_LOG(ERROR) << "invalid";
            state = WSParseState::ERROR;
            break;
        }

        WTF_LOG(INFO) << "valid";
    }

    return state;
}

WSParseState SessionImplEV::ReadHttpStatus(
        struct evbuffer *inBuf,
        int& httpMajorVersion,
        int& httpMinorVersion,
        int& statusCode)
{
    char *line = nullptr;
    SCOPE_GUARD {
        if (line) free(line);
    };
    size_t len;
    WTF_CHECK(inBuf);

    line = evbuffer_readln(inBuf, &len, EVBUFFER_EOL_CRLF);

    if (!line)
        return WSParseState::NEED_MORE;

    if (ParseHttpStatus(line, httpMajorVersion, httpMinorVersion, statusCode))
        return WSParseState::ERROR;

    return WSParseState::SUCCESS;
}

WSParseState SessionImplEV::ReadServerHandshakeReply(struct evbuffer *inBuf) {

    char *line = NULL;
    char *header_name = NULL;
    char *header_val = NULL;
    int majorVersion;
    int minorVersion;
    int statusCode;
    WSParseState parseState;
    WTF_CHECK(inBuf);

    WTF_LOG(INFO) << "Reading server handshake reply";

    switch (connectState_) {
        case WSConnectState::SENT_REQ: {
            httpHeaderFlags_ = 0;

            // Parse status line HTTP/1.1 200 OK...
            if ((parseState = ReadHttpStatus(inBuf, majorVersion, minorVersion, statusCode)) != WSParseState::SUCCESS)
                return parseState;

            WTF_LOG(INFO) << "HTTP/" << majorVersion <<"." << minorVersion << " " << statusCode;

            if ((majorVersion != 1) || (minorVersion != 1)) {
                WTF_LOG(ERROR) << wtf::FmtString("Server using unsupported HTTP "
                                     "version %d.%d", majorVersion, minorVersion);
                return WSParseState::ERROR;
            }

            if (statusCode != HTTP_STATUS_SWITCHING_PROTOCOLS_101) {
                // TODO: This must not be invalid. Could be a redirect for instance (add callback for this, so the user can decide to follow it... and also add auto follow support).
                WTF_LOG(ERROR) << wtf::FmtString("Invalid HTTP status code (%d)",
                                                 statusCode);
                return WSParseState::ERROR;
            }

            connectState_ = WSConnectState::PARSED_STATUS;
            // Fall through.
        }
        case WSConnectState::PARSED_STATUS: {
            WTF_LOG(INFO) << "Reading headers";

            // Read the HTTP headers.
            if ((parseState = ReadHttpHeaders(inBuf)) != WSParseState::SUCCESS) {
                return parseState;
            }

            WTF_LOG(INFO) << "Successfully parsed HTTP headers";

            connectState_ = WSConnectState::PARSED_HEADERS;
            // Fall through.
        }
        case WSConnectState::PARSED_HEADERS: {
            uint8_t flag = httpHeaderFlags_;
            WTF_LOG(INFO) << "Checking if we have all required headers:";

            if (!(flag & WS_HAS_VALID_UPGRADE_HEADER)) {
                WTF_LOG(ERROR) << "Missing Upgrade header";
                return WSParseState::ERROR;
            }

            if (!(flag & WS_HAS_VALID_CONNECTION_HEADER)) {
                WTF_LOG(ERROR) << "Missing Connection header";
                return WSParseState::ERROR;
            }

            if (!(flag & WS_HAS_VALID_WS_ACCEPT_HEADER)) {
                WTF_LOG(ERROR) << "Missing Sec-WebSocket-Accept header";
                return WSParseState::ERROR;
            }

            if (!(flag & WS_HAS_VALID_WS_PROTOCOL_HEADER)) {
                if (numSubprotocols_ > 0) {
                    WTF_LOG(WARNING) << "Server did not reply to my "
                                          "subprotocol request";
                }
            }

            WTF_LOG(INFO) << "Handshake complete";
            connectState_ = WSConnectState::HANDSHAKE_COMPLETE;
            break;
        }
        default: {
            WTF_LOG(ERROR) << "Incorrect connect state in server handshake response handler ";// << connectState_;
            return WSParseState::ERROR;
        }
    }

    return WSParseState::SUCCESS;
}

int SessionImplEV::OnSendMsgBegin(int binary) {
    WTF_CHECK(state_ != WSState::CONNECTED) << "message begin";

    WTF_LOG(DEBUG) << "Message begin";

    if (sendState_ != WSSendState::NONE) {
        return -1;
    }

    binaryMode_ = binary;

    wsHeader_ = {0};
    // FIN, RSVx bits are 0.
    wsHeader_.opcode = binaryMode_ ?
                       WSOpcode::BINARY_0X2 : WSOpcode::TEXT_0X1;

    sendState_ = WSSendState::MESSAGE_BEGIN;

    return 0;
}


int SessionImplEV::OnMsgFrameDataBegin(uint64_t datalen) {
    uint8_t header_buf[WS_HDR_MAX_SIZE];
    size_t header_len = 0;

    WTF_LOG(DEBUG) << wtf::FmtString("Message frame data begin, opcode 0x%x "
                           "(send header)", wsHeader_.opcode, datalen);

    if ((sendState_ != WSSendState::MESSAGE_BEGIN)
        && (sendState_ != WSSendState::IN_MESSAGE)) {
        WTF_LOG(ERROR) << "Incorrect state for frame data begin";
        return -1;
    }

    if (datalen > WS_MAX_PAYLOAD_LEN) {
        WTF_LOG(ERROR) << wtf::FmtString("Payload length (0x%x) larger than max allowed "
                                         "websocket payload (0x%x)",
                                         datalen, WS_MAX_PAYLOAD_LEN);
        return -1;
    }

    wsHeader_.maskBit = 0x1;
    wsHeader_.payloadLen = datalen;

    if (GetRandomMask((unsigned char *)&wsHeader_.mask, sizeof(uint32_t))
        != sizeof(uint32_t)) {
        return -1;
    }

    if (sendState_ == WSSendState::MESSAGE_BEGIN)
    {
        // Opcode will be set to either TEXT or BINARY here.
        WTF_CHECK((wsHeader_.opcode == WSOpcode::TEXT_0X1)
               || (wsHeader_.opcode == WSOpcode::BINARY_0X2));

        sendState_ = WSSendState::IN_MESSAGE;
    } else {
        // We've already sent frames.
        wsHeader_.opcode = WSOpcode::CONTINUATION_0X0;
    }

    PackHeader(wsHeader_, header_buf, sizeof(header_buf), &header_len);

    if (SendData((char *)header_buf, (uint64_t)header_len, 0)) {
        WTF_LOG(ERROR) << "Failed to send frame header";
        return -1;
    }

    return 0;
}

int SessionImplEV::OnMsgFrameDataSend(char *data, uint64_t datalen) {

    WTF_LOG(DEBUG) << "Message frame data send";

    if (sendState_ != WSSendState::IN_MESSAGE) {
        WTF_LOG(ERROR) << "Incorrect send state in frame data send";
        return -1;
    }

    // TODO: Don't touch original buffer as an option?
    if (wsHeader_.maskBit)
    {
        MaskPayload(wsHeader_.mask, data, datalen);
    }

    if (SendData(data, datalen, 1))
    {
        WTF_LOG(ERROR) << "Failed to send frame data";
        return -1;
    }

    return 0;
}

int SessionImplEV::OnMsgFrameSend(char *frame_data, uint64_t datalen) {
    WTF_CHECK(frame_data || (!frame_data && (datalen == 0)));
    WTF_CHECK(state_ != WSState::CONNECTED) << "message frame send";

    WTF_LOG(DEBUG) << "Message frame send";

    if ((sendState_ != WSSendState::MESSAGE_BEGIN)
        && (sendState_ != WSSendState::IN_MESSAGE)) {
        WTF_LOG(ERROR) << "Incorrect send state in message frame send";
        return -1;
    }

    if (OnMsgFrameDataBegin(datalen))
    {
        return -1;
    }

    if (OnMsgFrameDataSend(frame_data, datalen))
    {
        return -1;
    }

    return 0;
}

int SessionImplEV::OnSendMsgEnd() {

    WTF_LOG(DEBUG) << "Message end";

    if (sendState_ != WSSendState::IN_MESSAGE)
    {
        WTF_LOG(ERROR) << "Incorrect send state in message end";
        return -1;
    }

    // Write a frame with FIN bit set.
    wsHeader_.fin = 0x1;

    if (OnMsgFrameSend(NULL, 0)) {
        WTF_LOG(ERROR) << "Failed to send end of message frame";
        return -1;
    }

    sendState_ = WSSendState::NONE;

    return 0;
}

int SessionImplEV::SendMsgEx(char *msg, uint64_t len, int binary) {
    int saved_binary_mode;
    uint64_t curlen;
    uint64_t remaining;
    WTF_CHECK(state_ == WSState::CONNECTED) << "send message";

    WTF_LOG(INFO) << "Send message start";

    // Use _ws_send_frame_raw if we're not fragmenting the message.
    if ((len <= maxFrameSize_) || !maxFrameSize_) {
        return SendFrameRaw(binary ? WSOpcode::BINARY_0X2 : WSOpcode::TEXT_0X1,
                                  msg, len);
    }

    // Split the message into multiple frames.
    if (OnSendMsgBegin(binary)) {
        return -1;
    }

    curlen = (len > maxFrameSize_)
             ? maxFrameSize_ : len;
    remaining = len;

    do {
        curlen = (remaining > maxFrameSize_)
                 ? maxFrameSize_ : remaining;

        if (OnMsgFrameSend(msg, curlen)) {
            return -1;
        }

        msg += curlen;
        remaining -= curlen;
    }
    while (remaining > 0);

    if (OnSendMsgEnd()) {
        return -1;
    }

    WTF_LOG(INFO) << "Send message end";

    return 0;
}

int SessionImplEV::DoSendMsg(const std::string& msg) {
    int ret = 0;

    ret = SendMsgEx(const_cast<char*>(msg.c_str()), msg.size(), 0);

    return ret;
}

WSParseState SessionImplEV::UnpackHeader(size_t& headerLen, const unsigned char *b, size_t len)
{
    WTF_CHECK(b);

    wsHeader_ = {0};

    if (len < WS_HDR_MIN_SIZE) {
        return WSParseState::NEED_MORE;
    }

    // First byte.
    wsHeader_.fin	= (b[0] >> 7) & 0x1;
    wsHeader_.rsv1	= (b[0] >> 6) & 0x1;
    wsHeader_.rsv2	= (b[0] >> 5) & 0x1;
    wsHeader_.rsv3	= (b[0] >> 4) & 0x1;
    wsHeader_.opcode = (b[0] & 0xF);

    // Second byte.
    wsHeader_.maskBit = (b[1] >> 7) & 0x1;
    wsHeader_.payloadLen = (b[1] & 0x7F);
    headerLen = 2;
    // Get extended payload size if any (2 or 8 bytes).
    if (wsHeader_.payloadLen == 126) {
        if (len < (headerLen + 2)) {
            return WSParseState::NEED_MORE;
        } else {
            // 2 byte payload length.
            uint16_t *size_ptr = (uint16_t *)&b[2];
            wsHeader_.payloadLen = (uint64_t)ntohs(*size_ptr);
            headerLen += 2;
        }
    } else if (wsHeader_.payloadLen == 127) {
        if (len < (headerLen + 8)) {
            return WSParseState::NEED_MORE;
        } else {
            // 8 byte payload length.
            uint64_t *size_ptr = (uint64_t *)&b[2];
            wsHeader_.payloadLen = EncodeN2H64(*size_ptr);
            headerLen += 8;
        }
    }

    // Read masking key.
    if (wsHeader_.maskBit)
    {
        uint32_t *mask_ptr = (uint32_t *)&b[headerLen];
        // TODO: Hmm shouldn't it be ntohl here? (doesn't work with RFC examples though).
        wsHeader_.mask = (*mask_ptr);
        headerLen += 4;
    }

    return WSParseState::SUCCESS;
}

int SessionImplEV::ValidateHeader() {

    if (wsHeader_.rsv1 || wsHeader_.rsv2 || wsHeader_.rsv3) {
        WTF_LOG(ERROR) << "Protocol violation, reserve bit set";
        return -1;
    }

    if (IsReservedOpcode(wsHeader_.opcode)) {
        WTF_LOG(ERROR) << wtf::FmtString("Protocol violation, reserved opcode used %d (%s)",
                                         wsHeader_.opcode, Opcode2Str(wsHeader_.opcode));
        return -1;
    }

    if (IsControlOpcode(wsHeader_.opcode) && !wsHeader_.fin) {
        WTF_LOG(ERROR) << wtf::FmtString("Protocol violation, fragmented %s not allowed",
                                         Opcode2Str(wsHeader_.opcode));
        return -1;
    }

    if (wsHeader_.opcode == WSOpcode::CONTINUATION_0X0 &&
        inMsgFlag_) {
        WTF_LOG(ERROR) << "Got continuation frame when not in message";
        return -1;
    }

    // If we're in a message, we must either get a continuation frame
    // or an interjected control frame such as a ping.
    if (inMsgFlag_ &&
        (wsHeader_.opcode != WSOpcode::CONTINUATION_0X0 && !IsControlOpcode(wsHeader_.opcode))) {
        WTF_LOG(ERROR) << wtf::FmtString("Didn't get continuation frame when still in message. opcode %d (%s)",
                                         wsHeader_.opcode, Opcode2Str(wsHeader_.opcode));
        return -1;
    }

    return 0;
}


int SessionImplEV::SendData(const char *msg, uint64_t len, int no_copy) {
    // TODO: We supply a len of uint64_t, evbuffer_add uses size_t...

    WTF_LOG(INFO) << wtf::FmtString("Send the data (%llu bytes)", len);

    if (!bev_) {
        WTF_LOG(ERROR) << "Null bufferevent on send";
        return -1;
    }

    // If in no copy mode we only add a reference to the passed
    // buffer to the underlying bufferevent, and let it use the
    // user supplied cleanup function when it has sent the data.
    // (Note that the header will never be sent like this).
    if (no_copy && noCopyCleanupCb_)
    {
        if (evbuffer_add_reference(bufferevent_get_output(bev_),
                                   (void *)msg, (size_t)len, BuiltinNoCopyCleanupWrapper, (void *)this)) {
            WTF_LOG(ERROR) << "Failed to write reference to send buffer";
            return -1;
        }
    } else {
        // Send like normal (this will copy the data).
        if (evbuffer_add(bufferevent_get_output(bev_),
                         msg, (size_t)len)) {
            WTF_LOG(ERROR) << "Failed to write to send buffer";
            return -1;
        }
    }

    return 0;
}

int SessionImplEV::SendFrameRaw(WSOpcode opcode, char *data, uint64_t dataLen) {
    uint8_t header_buf[WS_HDR_MAX_SIZE];
    size_t header_len = 0;

    WTF_LOG(INFO) << wtf::FmtString(" Send frame raw 0x%x", opcode);

    if (sendState_ != WSSendState::NONE) {
        WTF_LOG(ERROR) << "Send state not none";
        return -1;
    }

    // All control frames MUST have a payload length of 125 bytes or less
    // and MUST NOT be fragmented.
    if (IsControlOpcode(opcode) && (dataLen > 125)) {
        WTF_LOG(ERROR) << "Control frame payload cannot be larger than 125 bytes";
        return -1;
    }

    // Pack and send header.
    {
        wsHeader_ = {0};

        wsHeader_.fin = 0x1;
        wsHeader_.opcode = opcode;

        if (dataLen > WS_MAX_PAYLOAD_LEN) {
            WTF_LOG(ERROR) << wtf::FmtString("Payload length (0x%x) larger than max allowed "
                                             "websocket payload (0x%x)",
                                             dataLen, WS_MAX_PAYLOAD_LEN);
            return -1;
        }

        wsHeader_.maskBit = 0x1;
        wsHeader_.payloadLen = dataLen;

        if (GetRandomMask(reinterpret_cast<unsigned char *>(&wsHeader_.mask), sizeof(wsHeader_.mask))
            != sizeof(uint32_t)) {
            return -1;
        }

        PackHeader(wsHeader_, header_buf, sizeof(header_buf), &header_len);

        if (SendData((char *)header_buf, (uint64_t)header_len, 0)) {
            WTF_LOG(ERROR) << "Failed to send frame header";
            return -1;
        }
    }

    // Send the data.
    {
        MaskPayload(wsHeader_.mask, data, dataLen);

        if (SendData(data, dataLen, 1)) {
            WTF_LOG(ERROR) << "Failed to send frame data";
            return -1;
        }
    }

    return 0;
}

int SessionImplEV::SendClose(WSCloseStatus statusCode, const char *reason, size_t reason_len) {
    char close_payload[WS_CONTROL_MAX_PAYLOAD_LEN];

    if (WS_IS_CLOSE_STATUS_NOT_USED(statusCode)) {
        WTF_LOG(ERROR) << wtf::FmtString("Invalid websocket close status code. "
                                         "Must be between 1000 and 4999. %u given",
                                         static_cast<uint16_t>(statusCode));
        return -1;
    }

    // (Status code is a uint16_t == 2 bytes)
    if ((reason_len + 2) > WS_CONTROL_MAX_PAYLOAD_LEN) {
        WTF_LOG(ERROR) << wtf::FmtString("Close reason too big to fit max control "
                                         "frame payload size %u + 2 byte status (max %d)",
                                         reason_len, WS_CONTROL_MAX_PAYLOAD_LEN);
        return -1;
    }

    *((uint16_t *)close_payload) = htons((uint16_t)statusCode);
    memcpy(&close_payload[2], reason, reason_len);

    if (SendFrameRaw(WSOpcode::CLOSE_0X8,
                     close_payload, reason_len + 2)) {
        WTF_LOG(ERROR) << "Failed to send close frame";
        return -1;
    }

    return 0;
}

int SessionImplEV::DoClose() {
    return CloseWithStatusReason(WSCloseStatus::NORMAL_1000, NULL, 0);
}

int SessionImplEV::CloseWithStatusReason(WSCloseStatus status, const char *reason, size_t reason_len) {
    struct timeval tv;

    WTF_LOG(INFO) << wtf::FmtString("Sending close frame %d, %*s",
                                    status, reason_len, reason);

    state_ = WSState::CLOSING;

    // The underlying TCP connection, in most normal cases, SHOULD be closed
    // first by the server, so that it holds the TIME_WAIT state and not the
    // client (as this would prevent it from re-opening the connection for 2
    // maximum segment lifetimes (2MSL), while there is no corresponding
    // server impact as a TIME_WAIT connection is immediately reopened upon
    // a new SYN with a higher seq number).  In abnormal cases (such as not
    // having received a TCP Close from the server after a reasonable amount
    // of time) a client MAY initiate the TCP Close.  As such, when a server
    // is instructed to _Close the WebSocket Connection_ it SHOULD initiate
    // a TCP Close immediately, and when a client is instructed to do the
    // same, it SHOULD wait for a TCP Close from the server.

    // Send a websocket close frame to the server.
    if (!sentCloseFlag_)
    {
        if (SendClose(status, reason, reason_len))
        {
            WTF_LOG(ERROR) << "Failed to send close frame";
            goto fail;
        }
    }

    sentCloseFlag_ = true;

    // Give the server time to initiate the closing of the
    // TCP session. Otherwise we'll force an unclean shutdown
    // ourselves.
    if (closeTimeoutEvent_) event_free(closeTimeoutEvent_);

    if (!(closeTimeoutEvent_ = evtimer_new(evBase_,
                                           OnCloseTimeout, (void *)this))) {
        WTF_LOG(ERROR) << "Failed to create close timeout event";
        goto fail;
    }

    tv.tv_sec = 3; // TODO: Let the user set this.
    tv.tv_usec = 0;

    if (evtimer_add(closeTimeoutEvent_, &tv)){
        WTF_LOG(ERROR) << "Failed to add close timeout event";
        goto fail;
    }

    return 0;

    fail:

    // If we fail to send the close frame, we do a TCP close
    // right away (unclean websocket close).

    if (closeTimeoutEvent_) {
        event_free(closeTimeoutEvent_);
        closeTimeoutEvent_ = NULL;
    }

    WTF_LOG(ERROR) << "Failed to send close frame, forcing unclean close";

    ShutDown();

    if (onClose_) {
        char reason[] = "Problem sending close frame";
        onClose_(WSCloseStatus::ABNORMAL_1006,
                     reason, sizeof(reason), nullptr);
    }

    return -1;
}

int SessionImplEV::CloseWithStatus(WSCloseStatus status) {
    return CloseWithStatusReason(status, NULL, 0);
}

void SessionImplEV::OnRecvMsgBegin() {

    WTF_LOG(INFO) << "Default message begin callback "
                     "(setup message buffer)";

    if (msg_) {
        if (evbuffer_get_length(msg_) != 0) {
            WTF_LOG(WARNING) << "Non-empty message buffer on new message";
        }

        evbuffer_free(msg_);
        msg_ = nullptr;
    }

    if (!(msg_ = evbuffer_new())) {
        // TODO: Close connection. Internal error error code.
        return;
    }
}

void SessionImplEV::OnMsgFrameBegin(void *arg) {
    WTF_LOG(INFO) << "Default message frame begin callback "
                     "(Sets up the frame data buffer)";

    // Setup the frame payload buffer.
    if (frameData_) {
        if (evbuffer_get_length(frameData_) != 0) {
            WTF_LOG(WARNING) << "Non-empty message buffer on new frame";
            // TODO: This should probably fail somehow...
        }

        // TODO: Should we really free it? Not just do evbuffer_remove with a NULL dest?
        evbuffer_free(frameData_);
        frameData_ = nullptr;
    }

    if (!(frameData_ = evbuffer_new())) {
        // TODO: Close connection. Internal error reason.
        return;
    }
}

int SessionImplEV::HandleFrameBegin() {
    WTF_LOG(INFO) << wtf::FmtString("Frame begin, opcode = %s", Opcode2Str(wsHeader_.opcode));

    recvFrameLen_ = 0;

    if (IsControlOpcode(wsHeader_.opcode)) {
        WTF_LOG(DEBUG) << "Control frame";
        memset(ctrlPayload_, 0, sizeof(ctrlPayload_));
        ctrlLen_ = 0;
        return 0;
    }

    WTF_LOG(DEBUG) << "Normal frame";

    // Normal frame.
    if (!inMsgFlag_) {
        inMsgFlag_ = true;
        utf8State_ = WS_UTF8_ACCEPT;
        msgIsBinary_ = (wsHeader_.opcode == WSOpcode::BINARY_0X2);

        WTF_LOG(DEBUG) << "Call message begin callback";
        OnRecvMsgBegin();
    }

    WTF_LOG(DEBUG) << "Call frame begin callback";
    OnMsgFrameBegin(nullptr);

    return 0;
}

int SessionImplEV::HandleCloseFrame()
{
    WTF_LOG(INFO) << "Close frame";

    serverCloseStatus_ = WSCloseStatus::NORMAL_1000;
    serverReason_ = "";

    if (closeTimeoutEvent_)
    {
        event_free(closeTimeoutEvent_);
        closeTimeoutEvent_ = NULL;
    }

    state_ = WSState::CLOSING;
    receivedCloseFlag_ = true;

    // The Close frame MAY contain a body (the "Application data" portion of
    // the frame) that indicates a reason for closing.
    // If there is a body, the first two bytes of
    // the body MUST be a 2-byte unsigned integer (in network byte order)
    // representing a status code
    if (ctrlLen_ > 0) {
        if (ctrlLen_ < 2) {
            WTF_LOG(ERROR) << "Close frame application data lacking "
                                 "status code";

            serverCloseStatus_ = WSCloseStatus::STATUS_CODE_EXPECTED_1005;

            CloseWithStatus(WSCloseStatus::PROTOCOL_ERR_1002);
            return 0;
        } else {
            WTF_LOG(DEBUG) << wtf::FmtString("Reading server close status and reason "
                                   " (payload length %lu)", ctrlLen_);

            serverCloseStatus_ =
                    (WSCloseStatus)ntohs(*((uint16_t *)ctrlPayload_));
            serverReason_ = &ctrlPayload_[2];
            serverReasonLen_ = ctrlLen_ - 2;
            serverReason_[serverReasonLen_] = '\0';

            WTF_LOG(INFO) << wtf::FmtString("Got close status %d, \"%s\"",
                      serverCloseStatus_,
                      serverReason_);

            if (!WS_IS_PEER_CLOSE_STATUS_VALID(serverCloseStatus_))
            {
                WTF_LOG(ERROR) << wtf::FmtString("Invalid close code from peer %d",
                                                 serverCloseStatus_);
                CloseWithStatus(WSCloseStatus::PROTOCOL_ERR_1002);
                return 0;
            }

            // Validate UTF8 text.
            utf8State_ = WS_UTF8_ACCEPT;
            ValidateUtf8(&utf8State_,
                             serverReason_, serverReasonLen_);

            if (utf8State_ == WS_UTF8_REJECT)
            {
                CloseWithStatus(WSCloseStatus::INCONSISTENT_DATA_1007);
                return 0;
            }
        }
    }

    // If an endpoint receives a Close frame and did not previously send a
    // Close frame, the endpoint MUST send a Close frame in response.  (When
    // sending a Close frame in response, the endpoint typically echos the
    // status code it received.)  It SHOULD do so as soon as practical.  An
    // endpoint MAY delay sending a Close frame until its current message is
    // sent (for instance, if the majority of a fragmented message is
    // already sent, an endpoint MAY send the remaining fragments before
    // sending a Close frame).  However, there is no guarantee that the
    // endpoint that has already sent a Close frame will continue to process
    // data.
    if (!sentCloseFlag_)
    {
        WTF_LOG(INFO) << wtf::FmtString("Echoing status code %d", serverCloseStatus_);
        return CloseWithStatusReason(serverCloseStatus_,
                                     serverReason_,
                                     serverReasonLen_);
    }

    return 0;
}

int SessionImplEV::HandlePingFrame()
{
    WTF_LOG(INFO) << "Ping frame";

    onPing_(ctrlPayload_, ctrlLen_, 1, NULL);

    return 0;
}

int SessionImplEV::HandlePongFrame()
{
    WTF_LOG(INFO) << "Pong frame";

    onPong_(ctrlPayload_, ctrlLen_, 0, NULL);

    return 0;
}

int SessionImplEV::HandleControlFrame()
{
    WTF_LOG(INFO) << "Control frame";

    WTF_CHECK(IsControlOpcode(wsHeader_.opcode));

    switch (wsHeader_.opcode)
    {
        case WSOpcode::CLOSE_0X8: return HandleCloseFrame();
        case WSOpcode::PONG_0XA: return HandlePongFrame();
        case WSOpcode::PING_0X9: return HandlePingFrame();
        default:
        case WSOpcode::CONTROL_RSV_0XB:
        case WSOpcode::CONTROL_RSV_0XC:
        case WSOpcode::CONTROL_RSV_0XD:
        case WSOpcode::CONTROL_RSV_0XE:
        case WSOpcode::CONTROL_RSV_0XF:
            WTF_LOG(ERROR) << wtf::FmtString("Got unknown control frame 0x%x", wsHeader_.opcode);
            return -1;
    }

    return 0;
}

void SessionImplEV::OnMsgFrame(char *payload, uint64_t len, void *arg) {

    WTF_LOG(INFO) << "Default message frame callback "
                           "(append data to message)";

    // Finally we append the frame payload to the message payload.
    evbuffer_add_buffer(msg_, frameData_);
}

void SessionImplEV::OnMsgFrameEnd(void *arg) {
    WTF_LOG(INFO) << "Default message frame end callback "
                           "(Calls the message frame callback)";

    {
        size_t frame_len = evbuffer_get_length(frameData_);
        const unsigned char *payload = evbuffer_pullup(frameData_, frame_len);

        WTF_LOG(DEBUG) << "Calling message frame callback";
        OnMsgFrame((char *)payload, frame_len, arg);
    }

    evbuffer_free(frameData_);
    frameData_ = nullptr;
}

void SessionImplEV::OnRecvMsgEnd(void *arg) {
    size_t len;
    unsigned char *payload;

    WTF_LOG(INFO) << "Default message end callback "
                           "(Calls the on message callback)";

    // Finalize the message by adding a null char.
    // TODO: No null for binary?
    evbuffer_add_printf(msg_, "");

    len = evbuffer_get_length(msg_);
    payload = evbuffer_pullup(msg_, len);

    WTF_LOG(DEBUG) << wtf::FmtString("Message received of length %lu:\n%s", len, payload);

    if (onMessage_.IsValid() && state_ != WSState::CLOSING) {
        WTF_LOG(DEBUG) << "Calling message callback";
        onMessage_((char *)payload, len,
                   msgIsBinary_, nullptr);
    } else {
        WTF_LOG(DEBUG) << "No message callback set, drop message";
    }

    evbuffer_free(msg_);
    msg_ = NULL;
}

int SessionImplEV::HandleFrameEnd()
{
    WTF_LOG(DEBUG) << wtf::FmtString("Frame end, opcode = %d", wsHeader_.opcode);

    if (IsControlOpcode(wsHeader_.opcode)) {
        hasHeader_ = false;
        return HandleControlFrame();
    }

    OnMsgFrameEnd(nullptr);

    //! fin=1 indicates is a final fragment in a message.
    if (wsHeader_.fin) {
        OnRecvMsgEnd(nullptr);
        inMsgFlag_ = false;
    }

    hasHeader_ = false;

    return 0;
}

void SessionImplEV::OnMsgFrameData(char *payload, uint64_t len, void *arg) {

    WTF_LOG(INFO) << "Default message frame data callback "
                           "(Append data to frame data buffer)";

    // TODO: Instead of using a copied payload buf here, we should
    // instead use evbuffer_add_buffer()
    evbuffer_add(frameData_, payload, (size_t)len);
}

int SessionImplEV::HandleFrameData(char *buf, size_t len)
{
    int ret = 0;
    WTF_LOG(INFO) << "Handle frame data";

    if (IsControlOpcode(wsHeader_.opcode)) {
        size_t total_len = (ctrlLen_ + len);

        if (total_len > WS_CONTROL_MAX_PAYLOAD_LEN) {
            WTF_LOG(ERROR) << wtf::FmtString("Control payload too big %u, only %u allowed",
                      total_len, WS_CONTROL_MAX_PAYLOAD_LEN);

            // Copy the remaining data into the buf.
            len = WS_CONTROL_MAX_PAYLOAD_LEN - ctrlLen_;
            // TODO: Set protocol violation error status here. (This will then be handled in the read callback)
            CloseWithStatus(WSCloseStatus::PROTOCOL_ERR_1002);
            ret = -1;
        }

        WTF_LOG(DEBUG) << wtf::FmtString("Append %lu bytes to ctrl payload[%lu]", len, ctrlLen_);
        memcpy(&ctrlPayload_[ctrlLen_], buf, len);
        ctrlLen_ += len;

        return ret;
    }

    OnMsgFrameData(buf, len, nullptr);

    return ret;
}

void SessionImplEV::ReadWebsocket(struct evbuffer *in)
{
    WTF_CHECK(bev_);
    WTF_CHECK(in);

    WTF_LOG(INFO) << "Read websocket data";

    while (evbuffer_get_length(in)) {
        // First read the websocket header.
        if (!hasHeader_) {
            size_t headerLen;
            ev_ssize_t bytesRead;
            char headerBuf[WS_HDR_MAX_SIZE];
            WSParseState state;

            WTF_LOG(DEBUG) << "Read websocket header";

            bytesRead = evbuffer_copyout(in, (void *)headerBuf,
                                          sizeof(headerBuf));

            WTF_LOG(DEBUG) << wtf::FmtString("Copied %d header bytes", bytesRead);

            state = UnpackHeader(headerLen, (unsigned char *)headerBuf, bytesRead);

            WTF_CHECK(state != WSParseState::USER_ABORT);

            // Look for protocol violations in the header.
            if (state != WSParseState::NEED_MORE && ValidateHeader()) {
                state = WSParseState::ERROR;
            }

            switch (state) {
                case WSParseState::SUCCESS:
                    hasHeader_ = true;

                    WTF_LOG(INFO) << wtf::FmtString("Got header (%lu bytes):\n"
                                                    "fin = %d, rsv = {%d,%d,%d}, mask_bit = %d, opcode = 0x%x (%s), "
                                                    "mask = %x, len = %d",
                                                    headerLen, wsHeader_.fin, wsHeader_.rsv1, wsHeader_.rsv2,
                                                    wsHeader_.rsv3, wsHeader_.maskBit, wsHeader_.opcode,
                                                    Opcode2Str(wsHeader_.opcode), wsHeader_.mask, wsHeader_.payloadLen);
                    // Remove header len bytes data from the beginning of input evbuffer.
                    if (evbuffer_drain(in, headerLen)) {
                        // TODO: Error! close
                        WTF_LOG(ERROR) << "Failed to drain header buffer";
                    }
                    break;
                case WSParseState::NEED_MORE:
                    WTF_LOG(INFO) << "Need more header data";
                    return;
                case WSParseState::ERROR:
                    WTF_LOG(ERROR) << "Error protocol violation in header";
                    CloseWithStatus(WSCloseStatus::PROTOCOL_ERR_1002);
                    return;
                case WSParseState::USER_ABORT:
                    // TODO: What to do here?
                    WTF_LOG(ERROR) <<  "User abort";
                    break;
            }

            HandleFrameBegin();
        }

        if (hasHeader_) {
            // We're in a frame.
            size_t recv_len = evbuffer_get_length(in);
            size_t remaining = (size_t)(wsHeader_.payloadLen - recvFrameLen_);

            WTF_LOG(INFO) << wtf::FmtString("In frame (remaining %u bytes of %u payload)",
                      remaining, wsHeader_.payloadLen);

            if (recv_len > remaining) {
                WTF_LOG(INFO) << wtf::FmtString("Received %u of %u remaining bytes", recv_len, remaining);
                recv_len = remaining;
            }

            if (remaining == 0) {
                HandleFrameEnd();
            } else {
                int bytes_read;
                char *buf = (char *)malloc(recv_len);

                // TODO: Maybe we should only do evbuffer_pullup here instead
                // and pass that pointer on instead.
                bytes_read = evbuffer_remove(in, buf, recv_len);
                recvFrameLen_ += bytes_read;

                if (bytes_read != recv_len) {
                    WTF_LOG(ERROR) << wtf::FmtString("Wanted to read %u but only got %d",
                              recv_len, bytes_read);
                }

                WTF_LOG(INFO) << wtf::FmtString("read: %d (%llu of %u bytes)",
                          bytes_read, recvFrameLen_, wsHeader_.payloadLen);

                if (wsHeader_.maskBit) {
                    UnmaskPayload(wsHeader_.mask, buf, bytes_read);
                }

                // Validate UTF8 text. Control frames are handled seperately.
                if (!msgIsBinary && !IsControlOpcode(wsHeader_.opcode)) {
                    WTF_LOG(INFO) << wtf::FmtString("About to validate UTF8, state = %d"
                                            " len = %d", utf8State_, bytes_read);

                    ValidateUtf8(&utf8State_,
                                     buf, bytes_read);

                    // Either the UTF8 is invalid, or a codepoint is not
                    // complete in the finish frame.
                    if ((utf8State_ == WS_UTF8_REJECT) || ((utf8State_ != WS_UTF8_ACCEPT) && (wsHeader_.fin))) {
                        WTF_LOG(ERROR) << "Invalid UTF8!";

                        CloseWithStatus(WSCloseStatus::INCONSISTENT_DATA_1007);
                    }

                    WTF_LOG(INFO) << wtf::FmtString("Validated UTF8, state = %d",
                                                    utf8State_);
                }

                if (HandleFrameData(buf, bytes_read)) {
                    // TODO: Raise protocol error via error cb.
                    // TODO: Close connection.
                    WTF_LOG(ERROR) << "Failed to handle frame data";
                } else {
                    // TODO: This is not hit in some cases.
                    WTF_LOG(INFO) << wtf::FmtString("recv_frame_len = %llu, payload_len = %u",
                              recvFrameLen_, wsHeader_.payloadLen);
                    // The entire frame has been received.
                    if (recvFrameLen_ == wsHeader_.payloadLen) {
                        HandleFrameEnd();
                    }
                }

                free(buf);
            }
        }
    }

    WTF_LOG(DEBUG) << wtf::FmtString("%lu bytes left after websocket read",
                                     evbuffer_get_length(in));
}
void SessionImplEV::Request(bufferevent* bev){

    WTF_LOG(INFO) << "request";

    auto out = bufferevent_get_output(bev);
    evbuffer_add_printf(out, "GET %s HTTP/1.1\r\n", uri_.c_str());
    evbuffer_add_printf(out, "Host:%s:%d\r\n",host_.c_str(), port_);
    evbuffer_add_printf(out, "Upgrade:websocket\r\n");
    evbuffer_add_printf(out, "Connection:upgrade\r\n");
    evbuffer_add_printf(out, "Sec-WebSocket-Key:%s\r\n", "dGhlIHNhbXBsZSBub25jZQ==");
    evbuffer_add_printf(out, "Sec-WebSocket-Version:13\r\n");
    evbuffer_add_printf(out, "Origin:http://%s:%d\r\n",host_.c_str(), port_); //missing this key will lead to 403 response.

    evbuffer_add_printf(out, "\r\n");
}

void SessionImplEV::ShutDown() {
    WTF_LOG(INFO) << "Websocket shutdown";

    if (connectTimeoutEvent_) {
        event_free(connectTimeoutEvent_);
        connectTimeoutEvent_ = NULL;
    }

#ifdef LIBWS_WITH_OPENSSL
    _ws_openssl_close(ws);
#endif

    if (bev_) {
        bufferevent_free(bev_);
        bev_ = NULL;
        WTF_LOG(INFO) << "Freed bufferevent";
    }

    // TODO: Only quit when the base has no more connections.
    //ws_base_quit(ws->ws_base, 1);
}

int SessionImplEV::SendHandshake(struct evbuffer *out) {
    WTF_CHECK(out);

    WTF_LOG(INFO) << "Start sending websocket handshake";

    if (GenerateHandshakeKey()) {
        WTF_LOG(ERROR) << "Failed to generate handshake key";
        return -1;
    }

    evbuffer_add_printf(out, "GET /%s HTTP/1.1\r\n", !uri_.empty() ? uri_.c_str() : "");
    evbuffer_add_printf(out, "Host: %s:%d\r\n",host_.c_str(), port_);
    evbuffer_add_printf(out, "Connection: Upgrade\r\n");
    evbuffer_add_printf(out, "Upgrade: websocket\r\n");
    evbuffer_add_printf(out, "Sec-Websocket-Version: 13\r\n");
    evbuffer_add_printf(out, "Sec-WebSocket-Key: %s\r\n", handshakeKeyBase64_.c_str());

    // missing this key maybe lead to 403 response.
    if (!origin_.empty()) {
        evbuffer_add_printf(out, "Origin: %s\r\n", origin_.c_str());
    }

    if (numSubprotocols_ > 0) {
        evbuffer_add_printf(out, "Sec-WebSocket-Protocol: %s",
                            subprotocols_[0].c_str());

        for (size_t i = 1; i < numSubprotocols_; i++) {
            evbuffer_add_printf(out, ", %s", subprotocols_[i].c_str());
        }
    }

    evbuffer_add_printf(out, "\r\n");

    connectState_ = WSConnectState::SENT_REQ;

    WTF_LOG(INFO) << evbuffer_pullup(out, evbuffer_get_length(out));

    // TODO: Sec-WebSocket-Extensions

    // TODO: Add custom headers.

    return 0;
}

std::string SessionImplEV::CalculateKeyHash(const char *handshake_key_base64)
{
    unsigned char keyHashSha1[20]; // SHA1, 160 bits = 20 bytes.
    constexpr char accept_key[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    std::string keyHash(std::string(handshake_key_base64).append(accept_key));

    SHA1(reinterpret_cast<const unsigned char *>(keyHash.c_str()),
         keyHash.size(), keyHashSha1);

    std::string KeyHashSha1Base64 = EncodeBase64(keyHashSha1, sizeof(keyHashSha1));

    return std::move(KeyHashSha1Base64);
}

int SessionImplEV::ValidateHttpHeader(uint8_t flag,
                                      const std::string& name, const std::string& val,
                                      const std::string& expectedName, const std::string& expected_val,
                                      int must_appear_only_once)
{
    if (CompareStrIgnoreCase(expectedName, name)) {
        if (must_appear_only_once && (httpHeaderFlags_ & flag)) {
            WTF_LOG(ERROR) << wtf::FmtString("%s must only appear once", expectedName.c_str());
            return -1;
        }

        if (!CompareStrIgnoreCase(expected_val, val)) {
            WTF_LOG(ERROR) << wtf::FmtString("%s header must contain \"%s\" "
                                 "but got \"%s\"", name.c_str(), expected_val.c_str(), val.c_str());
            return -1;
        }

        httpHeaderFlags_ |= flag;
    }

    return 0;
}
#if 0
int SessionImplEV::CheckServerProtocolList(const char *val)
{
    int ret = 0;
    size_t i;
    std::string s = val;
    char *v = s;
    char *prot = NULL;
    char *end = NULL;
    int found = 0;

    while ((prot = libws_strsep(&v, ",")) != NULL)
    {
        // Trim start.
        prot += strspn(prot, " ");

        // Trim end.
        ws_rtrim(prot);

        found = 0;

        for (i = 0; i < ws->num_subprotocols; i++)
        {
            if (!strcasecmp(ws->subprotocols[i], prot))
            {
                // TODO: Add subprotocol to negotiated list of sub protocols.
                // TODO: Maybe give the user a list of these in the connection callback?
                found = 1;
            }
        }

        if (!found)
        {
            ret = -1;
            break;
        }
    }

    _ws_free(s);

    return ret;
}
#endif
int SessionImplEV::ValidateHttpHeaders(const std::string& name, const std::string& val)
{

    // 1.  If the status code received from the server is not 101, the
    //    client handles the response per HTTP [RFC2616] procedures.  In
    //    particular, the client might perform authentication if it
    //    receives a 401 status code; the server might redirect the client
    //    using a 3xx status code (but clients are not required to follow
    //    them), etc.  Otherwise, proceed as follows.

    // 2. If the response lacks an |Upgrade| header field or the |Upgrade|
    //    header field contains a value that is not an ASCII case-
    //    insensitive match for the value "websocket", the client MUST
    //    _Fail the WebSocket Connection_.
    if (ValidateHttpHeader(WS_HAS_VALID_UPGRADE_HEADER, name, val,
                           "Upgrade", "websocket", 1)) {
        return -1;
    }

    // 3. If the response lacks a |Connection| header field or the
    //    |Connection| header field doesn't contain a token that is an
    //    ASCII case-insensitive match for the value "Upgrade", the client
    //    MUST _Fail the WebSocket Connection_.
    if (ValidateHttpHeader(WS_HAS_VALID_CONNECTION_HEADER, name, val,
                                 "Connection", "upgrade", 1)) {
        return -1;
    }

    // 4. If the response lacks a |Sec-WebSocket-Accept| header field or
    //    the |Sec-WebSocket-Accept| contains a value other than the
    //    base64-encoded SHA-1 of the concatenation of the |Sec-WebSocket-
    //    Key| (as a string, not base64-decoded) with the string "258EAFA5-
    //    E914-47DA-95CA-C5AB0DC85B11" but ignoring any leading and
    //    trailing whitespace, the client MUST _Fail the WebSocket
    //    Connection_.
    {
        std::string acceptKeyHash = CalculateKeyHash(handshakeKeyBase64_.c_str());
        if (acceptKeyHash.empty()) {
            return -1;
        }

        if (ValidateHttpHeader(WS_HAS_VALID_WS_ACCEPT_HEADER,
                               name, val, "Sec-WebSocket-Accept", acceptKeyHash.c_str(), 1)) {
            return -1;
        }
    }

    // 5.  If the response includes a |Sec-WebSocket-Extensions| header
    //    field and this header field indicates the use of an extension
    //    that was not present in the client's handshake (the server has
    //    indicated an extension not requested by the client), the client
    //    MUST _Fail the WebSocket Connection_.  (The parsing of this
    //    header field to determine which extensions are requested is
    //    discussed in Section 9.1.)
    //
    // Note that like other HTTP header fields, this header field MAY be
    // split or combined across multiple lines.  Ergo, the following are
    // equivalent:
    //
    //      Sec-WebSocket-Extensions: foo
    //      Sec-WebSocket-Extensions: bar; baz=2
    //
    // is exactly equivalent to
    //
    //      Sec-WebSocket-Extensions: foo, bar; baz=2
    //
    // Note that the order of extensions is significant.  Any interactions
    // between multiple extensions MAY be defined in the documents defining
    // the extensions.  In the absence of such definitions, the
    // interpretation is that the header fields listed by the client in its
    // request represent a preference of the header fields it wishes to use,
    // with the first options listed being most preferable.  The extensions
    // listed by the server in response represent the extensions actually in
    // use for the connection.  Should the extensions modify the data and/or
    // framing, the order of operations on the data should be assumed to be
    // the same as the order in which the extensions are listed in the
    // server's response in the opening handshake.
    //
    if (CompareStrIgnoreCase(name, "Sec-WebSocket-Extensions")) {
        //
        // The |Sec-WebSocket-Extensions| header field MAY appear multiple times
        // in an HTTP request (which is logically the same as a single
        // |Sec-WebSocket-Extensions| header field that contains all values.
        // However, the |Sec-WebSocket-Extensions| header field MUST NOT appear
        // more than once in an HTTP response.
        //
        {
            // TODO: Parse extension list here. Right now no extensions are supported, so fail by default.
            WTF_LOG(ERROR) << wtf::FmtString("The server wants to use an extension "
                                 "we didn't request: %s", val.c_str());
            // TODO: Set close reason here.
            return -1;
        }

        httpHeaderFlags_ |= WS_HAS_VALID_WS_EXT_HEADER;
    }

    // 6. If the response includes a |Sec-WebSocket-Protocol| header field
    //    and this header field indicates the use of a subprotocol that was
    //    not present in the client's handshake (the server has indicated a
    //    subprotocol not requested by the client), the client MUST _Fail
    //    the WebSocket Connection_.
    if (CompareStrIgnoreCase("Sec-WebSocket-Protocol", name)) {
        // The |Sec-WebSocket-Protocol| header field MAY appear multiple times
        // in an HTTP request (which is logically the same as a single
        // |Sec-WebSocket-Protocol| header field that contains all values).
        // However, the |Sec-WebSocket-Protocol| header field MUST NOT appear
        // more than once in an HTTP response.
        if (httpHeaderFlags_ & WS_HAS_VALID_WS_PROTOCOL_HEADER) {
            WTF_LOG(ERROR) << "Got more than one \"Sec-WebSocket-Protocol\""
                                 " header in HTTP response";

            httpHeaderFlags_ &= ~WS_HAS_VALID_WS_PROTOCOL_HEADER;
            return -1;
        }

        //if (_ws_check_server_protocol_list(ws, val))
        {
            WTF_LOG(ERROR) << wtf::FmtString("Server wanted to use a subprotocol we "
                                 "didn't request: %s", val.c_str());

            httpHeaderFlags_ &= ~WS_HAS_VALID_WS_PROTOCOL_HEADER;
            return -1;
        }

        httpHeaderFlags_ |= WS_HAS_VALID_WS_PROTOCOL_HEADER;
    }

    return 0;
}

void SessionImplEV::SetHandler(const ConnectHandler& onConnect) {
    onConnect_ = std::move(onConnect);
}
void SessionImplEV::SetHandler(const MessageHandler& onMessage) {
    onMessage_ = std::move(onMessage);
}
} // namespace ws