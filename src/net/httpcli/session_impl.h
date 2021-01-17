//
// Created by admin on 2021/1/17.
//

#ifndef TEST_SESSION_IMPL_H
#define TEST_SESSION_IMPL_H

#include "net/httpcli/url.h"
#include "net/httpcli/parameters.h"
#include "net/httpcli/header.h"
#include "net/httpcli/timeout.h"
#include "net/httpcli/authentication.h"
#include "net/httpcli/digest.h"
#include "net/httpcli/multipart.h"
#include "net/httpcli/maxRedirects.h"
#include "net/httpcli/cookies.h"
#include "net/httpcli/body.h"
#include "net/httpcli/handler.h"
#include "net/httpcli/method.h"
namespace HttpCli {
class SessionImpl {
public:

    virtual void SetUrl(const Url &url);

    virtual void SetParameters(const Parameters &parameters);

    virtual void SetParameters(Parameters &&parameters);

    virtual void SetHeader(const Header &header);

    virtual void SetTimeout(const Timeout &timeout);

    virtual void SetAuth(const authentication &auth);

    virtual void SetDigest(const Digest &auth);

    virtual void SetMultipart(Multipart &&multipart);

    virtual void SetMultipart(const Multipart &multipart);

    virtual void SetRedirect(const bool &redirect);

    virtual void SetMaxRedirects(const MaxRedirects &max_redirects);

    virtual void SetCookies(const Cookies &cookies);

    virtual void SetBody(body &&body);

    virtual void SetBody(const body &body);

    virtual void DoRequest() = 0;
};
} //namespace HttpCli

#endif //TEST_SESSION_IMPL_H
