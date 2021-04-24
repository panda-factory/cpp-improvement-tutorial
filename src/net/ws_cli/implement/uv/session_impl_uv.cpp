//
// Created by admin on 2021-03-06.
//

#include "session_impl_uv.h"

#include "core/logging.h"
#include "core/fmt_string.h"

#include "net/ws_cli/util.h"

namespace net {
namespace ws {

// | static | uv |
void SessionImplUV::OnAlloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    WTF_LOG(INFO) << "OnAlloc";
    void *ptr = malloc(suggested_size);
    WTF_CHECK(ptr != nullptr);
    buf->base = (char *)ptr;
    buf->len = suggested_size;
}
// | static | uv |
void SessionImplUV::OnClose(uv_handle_t* handle) {
    WTF_LOG(INFO) << "OnClose";
}
// | static | uv |
void SessionImplUV::OnConnect(uv_connect_t* conn, int status) {
    WTF_LOG(INFO) << "OnConnect: "<< status;
    WTF_CHECK(conn->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(conn->data);
    WTF_CHECK(thiz);

    uv_stream_t *stream = conn->handle; // same as &socket_
    uv_read_start(stream,  SessionImplUV::OnAlloc, SessionImplUV::OnRead);

    thiz->DoHandshake();
}
// | static | uv |
void SessionImplUV::OnRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    WTF_LOG(INFO) << "OnRead size=" << nread;
    WTF_CHECK(stream->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(stream->data);
    WTF_CHECK(thiz);

    if(nread > 0) {
        size_t nparsed = http_parser_execute(&(thiz->parser_), &(thiz->settings_), buf->base, nread);
        WTF_CHECK(nparsed == nread);
        thiz->connect_handler_();
    } else {
        /* EOF */
        uv_close((uv_handle_t*)stream, SessionImplUV::OnClose);
    }

    free(buf->base);
}
// | static | uv |
void SessionImplUV::OnResolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res) {
    WTF_LOG(INFO) << "OnResolved";
    WTF_CHECK(resolver->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(resolver->data);
    WTF_CHECK(thiz);

    if (status < 0) {
        WTF_LOG(ERROR) << wtf::FmtString("getaddrinfo callback error %s\n", uv_err_name(status));
        return;
    }

    char addr[17] = {'\0'};
    uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
    WTF_LOG(INFO) << addr;

    thiz->Connect((const struct sockaddr*) res->ai_addr);
}
// | static | uv |
void SessionImplUV::OnWrite(uv_write_t* req, int status) {
    WTF_LOG(INFO) << "OnWrite";
}
// | static | http-parser |
int SessionImplUV::OnHttpParserBody(http_parser *parser, const char *p, size_t len) {
    WTF_LOG(INFO) << "OnHttpParserBody";
    WTF_CHECK(parser->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(parser->data);
    WTF_CHECK(thiz);
    return 0;
    //return thiz->OnRecvBody(p, len);
}
// | static | http-parser |
int SessionImplUV::OnHttpParserMessageComplete(http_parser* parser) {
    WTF_LOG(INFO) << "OnHttpParserMessageComplete";
    WTF_CHECK(parser->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(parser->data);
    WTF_CHECK(thiz);
    return 0;
}
// | static | http-parser |
int SessionImplUV::OnHttpParserHeaderComplete(http_parser* parser) {
    WTF_LOG(INFO) << "OnHttpParserHeaderComplete";
    WTF_CHECK(parser->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(parser->data);
    WTF_CHECK(thiz);

    return 0;
}
// | static | http-parser |
int SessionImplUV::OnHttpParserHeaderField(http_parser* parser, const char* field, size_t len) {
    WTF_LOG(INFO) << "OnHttpParserHeaderField";
    WTF_CHECK(parser->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(parser->data);
    WTF_CHECK(thiz);
    thiz->field_ = std::string(field, len);

    return 0;
}
// | static | http-parser |
int SessionImplUV::OnHttpParserHeaderValue(http_parser* parser, const char* value, size_t len) {
    WTF_LOG(INFO) << "OnHttpParserHeaderValue";
    WTF_CHECK(parser->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(parser->data);
    WTF_CHECK(thiz);
    thiz->response_.header.insert_or_assign(std::move(thiz->field_), std::string(value, len));

    return 0;
}
// | static | http-parser |
int SessionImplUV::OnHttpParserStatus(http_parser* parser, const char* status, size_t len) {
    WTF_LOG(INFO) << "OnHttpParserStatus";
    WTF_CHECK(parser->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(parser->data);
    WTF_CHECK(thiz);

    thiz->response_.status = std::string(status, len);
    thiz->response_.statusCode = parser->status_code;
    return 0;
}

int SessionImplUV::DoHandshake() {
    WTF_CHECK(request_.method == Method("GET")) << "websocket protocol method must be GET.";

    const Url& url = request_.url;
    request_.header.insert_or_assign("Origin", wtf::FmtString("http://%s:%d", url.host.c_str(), url.port));
    request_.PreparePayload();
    //evbuffer_add_printf(out, "GET %s HTTP/1.1\r\n", uri_.c_str());
    //evbuffer_add_printf(out, "Host:%s:%d\r\n",host_.c_str(), port_);
    //evbuffer_add_printf(out, "Upgrade:websocket\r\n");
    //evbuffer_add_printf(out, "Connection:upgrade\r\n");
    //evbuffer_add_printf(out, "Sec-WebSocket-Key:%s\r\n", "dGhlIHNhbXBsZSBub25jZQ==");
    //evbuffer_add_printf(out, "Sec-WebSocket-Version:13\r\n");
    //evbuffer_add_printf(out, "Origin:http://%s:%d\r\n",host_.c_str(), port_); //missing this key will lead to 403 response.

    uv_buf_t data = uv_buf_init((char *)request_.raw.data(), request_.raw.size());
    Write(&data);
    return 0;
}

int SessionImplUV::Connect(const struct sockaddr* res) {
    WTF_LOG(INFO) << "Connect";
    uv_tcp_init(loop_, &socket_);

    socket_.data = this;
    connReq_.data = this;
    uv_tcp_connect(&connReq_, &socket_, res, SessionImplUV::OnConnect);
    return 0;
}

int SessionImplUV::DoClose() {
    return 0;
}
int SessionImplUV::DoInit() {

    WTF_LOG(INFO) << "DoInit";
    loop_ = uv_default_loop();

    http_parser_init(&parser_, HTTP_RESPONSE);
    parser_.data = this;
    settings_.on_status = SessionImplUV::OnHttpParserStatus;
    settings_.on_headers_complete = SessionImplUV::OnHttpParserHeaderComplete;
    settings_.on_message_complete = SessionImplUV::OnHttpParserMessageComplete;
    settings_.on_header_field = SessionImplUV::OnHttpParserHeaderField;
    settings_.on_header_value = SessionImplUV::OnHttpParserHeaderValue;
    settings_.on_body = SessionImplUV::OnHttpParserBody;

    request_.method = Method("GET");
    request_.header.insert_or_assign("Upgrade", "websocket");
    request_.header.insert_or_assign("Connection", "upgrade");
    request_.header.insert_or_assign("Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ==");
    request_.header.insert_or_assign("Sec-WebSocket-Version", "13");
    return 0;

}

int SessionImplUV::Resolve() {
    hints_.ai_family = PF_INET;
    hints_.ai_socktype = SOCK_STREAM;
    hints_.ai_protocol = IPPROTO_TCP;
    hints_.ai_flags = 0;
    resolver_.data = this;
    int r = uv_getaddrinfo(loop_, &resolver_, SessionImplUV::OnResolved, request_.url.host.c_str(), std::to_string(request_.url.port).c_str(), &hints_);

    if (r) {
        WTF_LOG(ERROR) << wtf::FmtString("getaddrinfo call error %s\n", uv_err_name(r));
        return 1;
    }
    return 0;
}

void SessionImplUV::Write(uv_buf_t* buffer) {
    WTF_LOG(INFO) << "Write";

    int r = uv_write(&write_, (uv_stream_t*)&socket_, buffer, 1, SessionImplUV::OnWrite);
    if(r < 0) {
        WTF_LOG(ERROR) << wtf::FmtString("write_to_socket error");
    }
}

int SessionImplUV::DoConnect() {
    Resolve();

    uv_run(loop_, UV_RUN_DEFAULT);

    WTF_LOG(INFO) << "Connect completed.";
    return 0;
}

int SessionImplUV::SendFrameRaw(WSOpcode opcode, char *data, uint64_t dataLen) {
    uint8_t header_buf[WS_HDR_MAX_SIZE];
    size_t header_len = 0;

    WTF_LOG(INFO) << wtf::FmtString("Send frame raw 0x%x", opcode);

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

        uv_buf_t uv_data = uv_buf_init((char *)header_buf, (uint64_t)header_len);
        Write(&uv_data);
    }

    // Send the data.
    {
        MaskPayload(wsHeader_.mask, data, dataLen);

        uv_buf_t uv_data = uv_buf_init(data, dataLen);
        //Write(&uv_data);
    }

    return 0;
}

int SessionImplUV::SendMsgEx(char *msg, uint64_t len, int binary) {
    int saved_binary_mode;
    uint64_t curlen;
    uint64_t remaining;
    //WTF_CHECK(state_ == WSState::CONNECTED) << "send message";

    WTF_LOG(INFO) << "Send message start";

    // Use _ws_send_frame_raw if we're not fragmenting the message.
    if ((len <= maxFrameSize_) || !maxFrameSize_) {
        return SendFrameRaw(binary ? WSOpcode::BINARY_0X2 : WSOpcode::TEXT_0X1,
                            msg, len);
    }

    WTF_LOG(INFO) << "Send message end";

    return 0;
}

int SessionImplUV::DoSendMsg(const std::string& msg) {
    return SendMsgEx(const_cast<char*>(msg.c_str()), msg.size(), 0);
}

void SessionImplUV::SetHandler(const ConnectHandler&& connect_handler) {
    connect_handler_ = std::move(connect_handler);
}

void SessionImplUV::SetHandler(const MessageHandler&& message_handler) {

}
void SessionImplUV::SetUrl(const Url &url) {
    struct http_parser_url parserUrl;
    http_parser_url_init(&parserUrl);

    const char *buf = url.url.data();
    http_parser_parse_url(buf, url.url.size(), 0, &parserUrl);

    request_.url = std::move(url);
    if ((1 << UF_SCHEMA) & parserUrl.field_set) {
        request_.url.schema = std::string(buf + parserUrl.field_data[UF_SCHEMA].off, parserUrl.field_data[UF_SCHEMA].len);
    }
    if ((1 << UF_HOST) & parserUrl.field_set) {
        request_.url.host = std::string(buf + parserUrl.field_data[UF_HOST].off, parserUrl.field_data[UF_HOST].len);
    }
    if ((1 << UF_PORT) & parserUrl.field_set) {
        request_.url.port = parserUrl.port;
    } else {
        request_.url.port = request_.url.schema == "http" ? 80 : 443;
    }
    if ((1 << UF_PATH) & parserUrl.field_set) {
        request_.url.path = std::string(buf + parserUrl.field_data[UF_PATH].off, parserUrl.field_data[UF_PATH].len);
    }
    if ((1 << UF_QUERY) & parserUrl.field_set) {
        request_.url.query = std::string(buf + parserUrl.field_data[UF_QUERY].off, parserUrl.field_data[UF_QUERY].len);
    }
    if ((1 << UF_FRAGMENT) & parserUrl.field_set) {
        request_.url.fragment = std::string(buf + parserUrl.field_data[UF_FRAGMENT].off, parserUrl.field_data[UF_FRAGMENT].len);
    }
}
} // namespace ws
} // namespace net