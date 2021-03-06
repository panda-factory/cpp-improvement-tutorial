//
// Created by admin on 2021/1/17.
//

#ifndef TEST_SESSION_IMPL_H
#define TEST_SESSION_IMPL_H

#include "net/http_cli/url.h"
#include "net/http_cli/parameters.h"
#include "net/http_cli/header.h"
#include "net/http_cli/timeout.h"
#include "net/http_cli/authentication.h"
#include "net/http_cli/digest.h"
#include "net/http_cli/multipart.h"
#include "net/http_cli/maxRedirects.h"
#include "net/http_cli/cookies.h"
#include "net/http_cli/body.h"
#include "net/http_cli/response.h"
#include "net/http_cli/method.h"
#include "net/http_cli/retries.h"
#include "net/http_cli/request.h"
#include "retries.h"

namespace http {
class SessionImpl {
public:
    virtual int DoInit() = 0;

    virtual void SetUrl(const Url &url) = 0;

    virtual void SetParameters(const Parameters &parameters) = 0;

    virtual void SetParameters(Parameters &&parameters) = 0;

    virtual void SetHandler(const ResponseHandler&& onResponse) = 0;

    virtual void SetHeader(const Header &header) = 0;

    virtual void SetTimeout(const Timeout &timeout) = 0;

    virtual void SetAuth(const authentication &auth) = 0;

    virtual void SetDigest(const Digest &auth) = 0;

    virtual void SetMethod(const Method& method) = 0;

    virtual void SetMultipart(Multipart &&multipart) = 0;

    virtual void SetMultipart(const Multipart &multipart) = 0;

    virtual void SetRedirect(const bool &redirect) = 0;

    virtual void SetMaxRedirects(const MaxRedirects &max_redirects) = 0;

    virtual void SetCookies(const Cookies &cookies) = 0;

    virtual void SetBody(body &&body) = 0;

    virtual void SetBody(const body &body) = 0;

    virtual void SetRetries(const Retries &retries) = 0;

    virtual void DoRequest() = 0;

    // Used in templated functions
    void SetOption(const Url &url);

    void SetOption(const Parameters &parameters);

    void SetOption(Parameters &&parameters);

    void SetOption(const Header &header);

    void SetOption(const Timeout &timeout);

    void SetOption(const authentication &auth);

    void SetOption(const Digest &auth);

    void SetOption(const Method &method);

    void SetOption(Multipart &&multipart);

    void SetOption(const Multipart &multipart);

    void SetOption(const bool &redirect);

    void SetOption(const MaxRedirects &max_redirects);

    void SetOption(const Cookies &cookies);

    void SetOption(body &&body);

    void SetOption(const body &body);

    void SetOption(const ResponseHandler&& onResponse);

    void SetOption(const Retries&& retries);

    virtual ~SessionImpl() {}
};
} //namespace http

#endif //TEST_SESSION_IMPL_H
