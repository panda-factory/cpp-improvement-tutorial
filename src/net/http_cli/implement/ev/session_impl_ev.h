//
// Created by admin on 2021/1/21.
//

#ifndef TEST_SESSION_IMPL_EV_H
#define TEST_SESSION_IMPL_EV_H

#include "net/http_cli/session_impl.h"

#include <event2/http.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

namespace http {
class SessionImplEV : public SessionImpl {
public:

    inline void Get();

    const Url& url() const;

    void OnResponse(Response&& response);

    bool DoInit() override;

    void SetUrl(const Url &url) override;

    void SetParameters(const Parameters &parameters) override;

    void SetParameters(Parameters &&parameters) override;

    void SetHandler(const ResponseHandler &&onResponse) override;

    void SetHeader(const Header &header) override;

    void SetTimeout(const Timeout &timeout) override;

    void SetAuth(const authentication &auth) override;

    void SetDigest(const Digest &auth) override;

    void SetMethod(const Method &method) override;

    void SetMultipart(Multipart &&multipart) override;

    void SetMultipart(const Multipart &multipart) override;

    void SetRedirect(const bool &redirect) override;

    void SetRetries(const Retries &retries) override;

    void SetMaxRedirects(const MaxRedirects &max_redirects) override;

    void SetCookies(const Cookies &cookies) override;

    void SetBody(body &&body) override;

    void SetBody(const body &body) override;

    void DoRequest() override;

    SessionImplEV();

    ~SessionImplEV();
private:
    static int VerifyCertCallback(X509_STORE_CTX *x509_ctx, void *arg);
    int AddCertForStore(X509_STORE *store, const std::string& name);
    void InitEventConnect();
    void HandleOpensslError(const std::string& func);


    bool ignoreCert_ = false;
    size_t retries_ = 0;
    std::string errmsg_;
    ResponseHandler responseHandler_;
    struct evhttp_request* evReq_ = nullptr;
    struct event_base *evBase_ = nullptr;
    struct evhttp_connection* evConn_ = nullptr;
    struct bufferevent *evBuffer_;
    Url url_;
    Timeout timeout_;
    SSL_CTX *sslContext_ = nullptr;
    SSL *ssl_ = NULL;
};
} //namespace http
#endif //TEST_SESSION_IMPL_EV_H
