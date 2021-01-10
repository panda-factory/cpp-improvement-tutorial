//
// Created by admin on 2021/1/10.
//

#ifndef TEST_SESSIONIMPL_H
#define TEST_SESSIONIMPL_H

#include "net/httpcli/Url.h"
#include "net/httpcli/Parameters.h"
#include "net/httpcli/Header.h"
#include "net/httpcli/Timeout.h"
#include "net/httpcli/Authentication.h"
#include "net/httpcli/Digest.h"
#include "net/httpcli/Multipart.h"
#include "net/httpcli/MaxRedirects.h"
#include "net/httpcli/Cookies.h"
#include "net/httpcli/Body.h"
#include "net/httpcli/Handler.h"
namespace HttpCli {
class SessionImpl {
public:

    void SetUrl(const Url &url);

    void SetParameters(const Parameters &parameters);

    void SetParameters(Parameters &&parameters);

    void SetHeader(const Header &header);

    void SetTimeout(const Timeout &timeout);

    void SetAuth(const Authentication &auth);

    void SetDigest(const Digest &auth);

    void SetMultipart(Multipart &&multipart);

    void SetMultipart(const Multipart &multipart);

    void SetRedirect(const bool &redirect);

    void SetMaxRedirects(const MaxRedirects &max_redirects);

    void SetCookies(const Cookies &cookies);

    void SetBody(Body &&body);

    void SetBody(const Body &body);
};
} //namespace HttpCli

#endif //TEST_SESSIONIMPL_H
