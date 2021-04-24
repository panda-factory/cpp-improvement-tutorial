//
// Created by admin on 2021-03-06.
//

#ifndef TEST_SESSION_IMPL_UV_H
#define TEST_SESSION_IMPL_UV_H

#include "net/ws_cli/session_impl.h"
#include "net/ws_cli/response.h"
#include "net/ws_cli/ws_state.h"
#include "net/ws_cli/ws_types.h"
#include "net/ws_cli/ws_header.h"
#include "uv.h"
#include "http_parser.h"

namespace net {
namespace ws {

class SessionImplUV : public SessionImpl {
public:
    int DoClose() override;
    int DoInit() override;
    int DoConnect() override;
    int DoSendMsg(const std::string& msg) override;

    void SetHandler(const ConnectHandler&& connect_handler) override;
    void SetHandler(const MessageHandler&& message_handler) override;
    void SetUrl(const Url &url) override;

private:
    static void OnAlloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
    static void OnClose(uv_handle_t* handle);
    static void OnConnect(uv_connect_t* conn, int status);
    static void OnRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
    static void OnResolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res);
    static void OnWrite(uv_write_t* req, int status);
    static int OnHttpParserBody(http_parser *parser, const char *p, size_t len);
    static int OnHttpParserHeaderComplete(http_parser* parser);
    static int OnHttpParserMessageComplete(http_parser* parser);
    static int OnHttpParserHeaderField(http_parser* parser, const char* header, size_t len);
    static int OnHttpParserHeaderValue(http_parser* parser, const char* value, size_t len);
    static int OnHttpParserStatus(http_parser* parser, const char* status, size_t len);

    int Connect(const struct sockaddr* res);
    int DoHandshake();
    int Resolve();
    int SendFrameRaw(WSOpcode opcode, char *data, uint64_t dataLen);
    int SendMsgEx(char *msg, uint64_t len, int binary);
    void Write(uv_buf_t* buffer);

    std::string field_;
    Request request_;
    Response response_;
    ConnectHandler connect_handler_;
    // | ws |
    WSState state_;
    WSSendState sendState_;
    WSHeader wsHeader_;
    uint64_t maxFrameSize_;    ///< The max frame size to allow before chunking.

    // | libuv |
    struct addrinfo hints_;
    uv_loop_t* loop_;
    uv_getaddrinfo_t resolver_;
    uv_connect_t connReq_;
    uv_tcp_t socket_;
    uv_write_t write_;
    http_parser parser_;
    http_parser_settings settings_;
};
} // namespace ws
} // namespace net

#endif //TEST_SESSION_IMPL_UV_H
