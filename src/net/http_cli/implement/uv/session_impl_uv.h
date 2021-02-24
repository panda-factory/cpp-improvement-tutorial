//
// Created by admin on 2021/2/20.
//

#ifndef TEST_SESSION_IMPL_UV_H
#define TEST_SESSION_IMPL_UV_H

#include "net/http_cli/session_impl.h"
#include "http_parser.h"
#include "uv.h"

namespace http {
class SessionImplUV : public SessionImpl {
public:
    int DoInit();

    void DoRequest();

    void SetUrl(const Url &url);

    void SetParameters(const Parameters &parameters);

    void SetParameters(Parameters &&parameters);

    void SetHandler(const ResponseHandler&& onResponse);

    void SetHeader(const Header &header);

    void SetTimeout(const Timeout &timeout);

    void SetAuth(const authentication &auth);

    void SetDigest(const Digest &auth);

    void SetMethod(const Method& method);

    void SetMultipart(Multipart &&multipart);

    void SetMultipart(const Multipart &multipart);

    void SetRedirect(const bool &redirect);

    void SetMaxRedirects(const MaxRedirects &max_redirects);

    void SetCookies(const Cookies &cookies);

    void SetBody(body &&body);

    void SetBody(const body &body);

    void SetRetries(const Retries &retries);
private:
    static void OnAlloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
    static void OnClose(uv_handle_t* handle);
    static void OnConnect(uv_connect_t* req, int status);
    static int OnHttpParserBody(http_parser *parser, const char *p, size_t len);
    static int OnHttpParserHeaderComplete(http_parser* parser);
    static int OnHttpParserMessageComplete(http_parser* parser);
    static int OnHttpParserHeaderField(http_parser* parser, const char* header, size_t len);
    static int OnHttpParserHeaderValue(http_parser* parser, const char* value, size_t len);
    static int OnHttpParserStatus(http_parser* parser, const char* status, size_t len);
    static void OnResolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res);
    static void OnWrite(uv_write_t* req, int status);
    static void OnRead(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf);

    int Connect(const struct sockaddr*);
    int OnRecvBody(const char *p, size_t len);
    int OnRecvComplete();
    void OnResponse(Response&& response);
    int Resolve();
    int Write(uv_stream_t * stream);

    std::string field_;
    Request request_;
    std::ostringstream bodyStream_;
    Response response_;
    ResponseHandler responseHandler_;

    // | Field |
    struct addrinfo hints_;
    uv_loop_t* loop_;
    uv_getaddrinfo_t resolver_;
    uv_connect_t connReq_;
    uv_tcp_t socket_;
    uv_write_t write_;
    http_parser parser_;
    http_parser_settings settings_;
};
} // namespace http

#endif //TEST_SESSION_IMPL_UV_H
