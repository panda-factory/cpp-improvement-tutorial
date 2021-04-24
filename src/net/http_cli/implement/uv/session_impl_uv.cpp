//
// Created by admin on 2021/2/20.
//

#include "session_impl_uv.h"

#include "core/logging.h"
#include "core/fmt_string.h"

#if __ENABLE_HTTPS__
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>

#ifdef  __cplusplus
extern "C" {
#endif // __cplusplus
#include <openssl/applink.c>
#ifdef  __cplusplus
}
#endif // __cplusplus
#endif // __ENABLE_HTTPS__

namespace net {
namespace http {
namespace {

#define WHERE_INFO(ssl, w, flag, msg) { \
    if(w & flag) { \
      printf("\t"); \
      printf(msg); \
      printf(" - %s ", SSL_state_string(ssl)); \
      printf(" - %s ", SSL_state_string_long(ssl)); \
      printf("\n"); \
    }\
 }
// INFO CALLBACK
void dummy_ssl_info_callback(const SSL* ssl, int where, int ret) {
    if(ret == 0) {
        printf("dummy_ssl_info_callback, error occured.\n");
        return;
    }
    WHERE_INFO(ssl, where, SSL_CB_LOOP, "LOOP");
    WHERE_INFO(ssl, where, SSL_CB_EXIT, "EXIT");
    WHERE_INFO(ssl, where, SSL_CB_READ, "READ");
    WHERE_INFO(ssl, where, SSL_CB_WRITE, "WRITE");
    WHERE_INFO(ssl, where, SSL_CB_ALERT, "ALERT");
    WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_DONE, "HANDSHAKE DONE");
}
// MSG CALLBACK
void dummy_ssl_msg_callback(
        int writep
        ,int version
        ,int contentType
        ,const void* buf
        ,size_t len
        ,SSL* ssl
        ,void *arg
) {
    printf("\tMessage callback with length: %zu\n", len);
}

// VERIFY
int dummy_ssl_verify_callback(int ok, X509_STORE_CTX* store) {
    char buf[256];
    X509* err_cert;
    err_cert = X509_STORE_CTX_get_current_cert(store);
    int err = X509_STORE_CTX_get_error(store);
    int depth = X509_STORE_CTX_get_error_depth(store);
    X509_NAME_oneline(X509_get_subject_name(err_cert), buf, 256);

    BIO* outbio = BIO_new_fp(stdout, BIO_NOCLOSE);
    X509_NAME* cert_name = X509_get_subject_name(err_cert);
    X509_NAME_print_ex(outbio, cert_name, 0, XN_FLAG_MULTILINE);
    BIO_free_all(outbio);
    WTF_LOG(INFO) << wtf::FmtString("\tssl_verify_callback(), ok: %d, error: %d, depth: %d, name: %s\n", ok, err, depth, buf);

    return 1;  // We always return 1, so no verification actually
}
}
// | static | uv |
void SessionImplUV::OnAlloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    WTF_LOG(INFO) << "OnAlloc";
    void *ptr = malloc(suggested_size);
    WTF_CHECK(ptr != nullptr);
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

    uv_stream_t *stream = conn->handle; // same as &socket_
    uv_read_start(stream,  SessionImplUV::OnAlloc, SessionImplUV::OnRead);

    if (thiz->request_.url.IsHttps()) {
#if __ENABLE_HTTPS__
        // Https protocol need to do handshake firstly with server after socket connect.
        int ret = SSL_do_handshake(thiz->ssl_);
        if(ret < 0) {
            thiz->HandleError(ret);
        }
#endif // __ENABLE_HTTPS__
    } else {
        thiz->request_.PreparePayload();
        uv_buf_t data = uv_buf_init((char *)thiz->request_.raw.data(), thiz->request_.raw.size());
        thiz->Write(&data);
    }

}
// | static | uv |
void SessionImplUV::OnClose(uv_handle_t* handle) {
    WTF_LOG(INFO) << "OnClose";
}
// | static | uv |
void SessionImplUV::OnRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    WTF_LOG(INFO) << "OnRead size=" << nread;
    WTF_CHECK(stream->data);
    SessionImplUV* thiz = static_cast<SessionImplUV*>(stream->data);
    WTF_CHECK(thiz);

    if(nread > 0) {
        if (thiz->request_.url.IsHttps()) {
#if __ENABLE_HTTPS__
            thiz->ReadFromSSL(buf, nread);
#endif // __ENABLE_HTTPS__
        } else {
            size_t nparsed = http_parser_execute(&(thiz->parser_), &(thiz->settings_), buf->base, nread);
            WTF_CHECK(nparsed == nread);
        }
    } else {
        thiz->OnRecvComplete();
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

#if __ENABLE_HTTPS__
void SessionImplUV::ReadFromSSL(const uv_buf_t *buf, ssize_t raw_size) {
    BIO_write(read_bio_, buf->base, raw_size);
    if (!SSL_is_init_finished(ssl_)) {
        int ret = SSL_connect(ssl_);
        if (ret < 0) {
            // handshake has not completed,
            HandleError(ret);
        } else {
            request_.PreparePayload();
            ret = SSL_write(ssl_, (char *) request_.raw.data(), request_.raw.size());
            if (ret <= 0) {
                HandleError(ret);
            }
            FlushReadBio();
        }
    } else {
        uv_buf_t ssl_buff;
        SessionImplUV::OnAlloc(nullptr, raw_size, &ssl_buff);
        uv_buf_init(ssl_buff.base, ssl_buff.len);
        int size = SSL_read(ssl_, ssl_buff.base, ssl_buff.len);
        if (size != -1) {
            size_t nparsed = http_parser_execute(&(parser_), &(settings_), ssl_buff.base, size);
            WTF_CHECK(nparsed == size)
            << wtf::FmtString("buffer size=%u, SSL_read size=%d, http_parse size=%d", raw_size, size, nparsed);
        }
    }
}
#endif // __ENABLE_HTTPS__

void SessionImplUV::Write(uv_buf_t* buffer) {
    WTF_LOG(INFO) << "Write";

    int r = uv_write(&write_, (uv_stream_t*)&socket_, buffer, 1, SessionImplUV::OnWrite);
    if(r < 0) {
        WTF_LOG(ERROR) << wtf::FmtString("write_to_socket error");
    }
}

int SessionImplUV::Connect(const struct sockaddr* res) {
    WTF_LOG(INFO) << "Connect";
    uv_tcp_init(loop_, &socket_);

    socket_.data = this;
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

#if __ENABLE_HTTPS__
    // Initialize SSL
    SSL_library_init();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    ssl_ctx_ = SSL_CTX_new(SSLv23_client_method());
    SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_SSLv2);
    //bio_stderr_ = BIO_new_fp(stderr, BIO_NOCLOSE);
    ssl_ = SSL_new(ssl_ctx_);
    read_bio_ = BIO_new(BIO_s_mem());
    write_bio_ = BIO_new(BIO_s_mem());
    SSL_set_bio(ssl_, read_bio_, write_bio_);
    SSL_set_connect_state(ssl_); //set to client mode

    SSL_CTX_set_verify(ssl_ctx_, SSL_VERIFY_PEER, dummy_ssl_verify_callback); // our callback always returns true, so no validation
    //SSL_CTX_set_info_callback(ssl_ctx_, dummy_ssl_info_callback);  // for debug
    SSL_CTX_set_msg_callback(ssl_ctx_, dummy_ssl_msg_callback);
#endif

    return 0;
}

void SessionImplUV::FlushReadBio() {
    char buf[1024*16];
    int bytes_read = 0;
    while((bytes_read = BIO_read(write_bio_, buf, sizeof(buf))) > 0) {
        uv_buf_t buffer = uv_buf_init((char *)buf, bytes_read);
        Write(&buffer);
    }
}

void SessionImplUV::HandleError(int result) {
    int err = SSL_get_error(ssl_, result);
    WTF_LOG(ERROR) << "TLS/SSL connect fail: " << err;
    if (err == SSL_ERROR_WANT_READ) { // wants to read from bio
        FlushReadBio();
    } else {
        WTF_LOG(FATAL) << "Need to specify more error code.";
    }
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
} // namespace net