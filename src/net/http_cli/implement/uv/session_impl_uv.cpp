//
// Created by admin on 2021/2/20.
//

#include "session_impl_uv.h"

#include "core/logging.h"
#include "core/fmt_string.h"

namespace http {
// | static | uv |
void SessionImplUV::OnAlloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    WTF_LOG(INFO) << "OnAlloc";
    void *ptr;
    ptr = malloc(suggested_size);
    buf->base = (char *)ptr;
    buf->len = suggested_size;
}
// | static | uv |
void SessionImplUV::OnWrite(uv_write_t* req, int status) {
    WTF_LOG(INFO) << "OnWrite";
}
// | static | uv |
void SessionImplUV::OnConnect(uv_connect_t* conn, int status) {
    WTF_LOG(INFO) << "OnConnect: "<< status;
    WTF_CHECK(conn->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(conn->data);
    WTF_CHECK(thiz);

    uv_stream_t *stream = conn->handle;
    stream->data = conn->data;
    thiz->Write(stream);
    uv_read_start(stream,  SessionImplUV::OnAlloc, SessionImplUV::OnRead);
}
// | static | uv |
void SessionImplUV::OnClose(uv_handle_t* handle) {
    WTF_LOG(INFO) << "OnClose";
}
// | static | uv |
void SessionImplUV::OnRead(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf) {
    WTF_LOG(INFO) << "OnRead";
    WTF_CHECK(tcp->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(tcp->data);
    WTF_CHECK(thiz);

    if(nread > 0) {
        size_t nparsed = http_parser_execute(&(thiz->parser_), &(thiz->settings_), buf->base, nread);
        WTF_CHECK(nparsed == nread);
    } else {
        thiz->OnRecvComplete();
        /* EOF */
        uv_close((uv_handle_t*)tcp, SessionImplUV::OnClose);
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
// | static | http-parser |
int SessionImplUV::OnHttpParserBody(http_parser *parser, const char *p, size_t len) {
    WTF_LOG(INFO) << "OnHttpParserBody";
    WTF_CHECK(parser->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(parser->data);
    WTF_CHECK(thiz);

    return thiz->OnRecvBody(p, len);
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
    thiz->request_.header.insert_or_assign(std::move(thiz->field_), std::string(value, len));

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

int SessionImplUV::Write(uv_stream_t * stream) {
    WTF_LOG(INFO) << "Write";
    request_.PreparePayload();

    uv_buf_t http = uv_buf_init((char *)request_.raw.data(), request_.raw.size());

    return uv_write(&write_, stream, &http, 1, SessionImplUV::OnWrite);
}

int SessionImplUV::Connect(const struct sockaddr* res) {
    WTF_LOG(INFO) << "Connect";
    uv_tcp_init(loop_, &socket_);

    connReq_.data = this;
    uv_tcp_connect(&connReq_, &socket_, res, SessionImplUV::OnConnect);
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
    return 0;
}

int SessionImplUV::OnRecvBody(const char *p, size_t len) {
    bodyStream_ << std::string(p, len);
    return 0;
}

int SessionImplUV::OnRecvComplete() {
    response_.body = std::move(bodyStream_.str());
    response_.url = std::move(request_.url);
    response_.header = std::move(request_.header);
    OnResponse(std::move(response_));
    return 0;
}

void SessionImplUV::OnResponse(Response&& response) {
    responseHandler_(std::move(response));
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

void SessionImplUV::DoRequest() {
    Resolve();

    uv_run(loop_, UV_RUN_DEFAULT);

    WTF_LOG(INFO) << "Request completed.";
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

void SessionImplUV::SetParameters(const Parameters &parameters) {

}

void SessionImplUV::SetParameters(Parameters &&parameters) {

}

void SessionImplUV::SetHandler(const ResponseHandler&& onResponse) {
    responseHandler_ = std::move(onResponse);
}

void SessionImplUV::SetHeader(const Header &headers) {
}

void SessionImplUV::SetTimeout(const Timeout &timeout) {
}

void SessionImplUV::SetAuth(const authentication &auth) {

}

void SessionImplUV::SetDigest(const Digest &auth) {

}

void SessionImplUV::SetMethod(const Method &method) {
    request_.method = method;
}

void SessionImplUV::SetMultipart(Multipart &&multipart) {

}

void SessionImplUV::SetMultipart(const Multipart &multipart) {

}

void SessionImplUV::SetRedirect(const bool &redirect) {

}

void SessionImplUV::SetRetries(const Retries &retries) {
}

void SessionImplUV::SetMaxRedirects(const MaxRedirects &max_redirects) {

}

void SessionImplUV::SetCookies(const Cookies &cookies) {

}

void SessionImplUV::SetBody(body &&body) {

}

void SessionImplUV::SetBody(const body &body) {

}

} // namespace http