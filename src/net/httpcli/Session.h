//
// Created by admin on 2021/1/10.
//

#ifndef TEST_SESSION_H
#define TEST_SESSION_H

#include "net/httpcli/SessionImpl.h"

namespace HttpCli {

class Session {
public:

    inline void SetUrl(const Url &url);

    inline void SetParameters(const Parameters &parameters);

    inline void SetParameters(Parameters &&parameters);

    inline void SetHeader(const Header &header);

    inline void SetTimeout(const Timeout &timeout);

    inline void SetAuth(const Authentication &auth);

    inline void SetDigest(const Digest &auth);

    inline void SetMultipart(Multipart &&multipart);

    inline void SetMultipart(const Multipart &multipart);

    inline void SetRedirect(const bool &redirect);

    inline void SetMaxRedirects(const MaxRedirects &max_redirects);

    inline void SetCookies(const Cookies &cookies);

    inline void SetBody(Body &&body);

    inline void SetBody(const Body &body);

    // Used in templated functions
    inline void SetOption(const Url &url);

    inline void SetOption(const Parameters &parameters);

    inline void SetOption(Parameters &&parameters);

    inline void SetOption(const Header &header);

    inline void SetOption(const Timeout &timeout);

    inline void SetOption(const Authentication &auth);

    inline void SetOption(const Digest &auth);

    inline void SetOption(Multipart &&multipart);

    inline void SetOption(const Multipart &multipart);

    inline void SetOption(const bool &redirect);

    inline void SetOption(const MaxRedirects &max_redirects);

    inline void SetOption(const Cookies &cookies);

    inline void SetOption(Body &&body);

    inline void SetOption(const Body &body);

    template<class H>
    inline void SetOption(const Handler <H> &&on_response);

    inline void DELETE_();

    inline void GET();

    inline void HEAD();

    inline void OPTIONS();

    inline void PATCH();

    inline void POST();

    inline void PUT();

    Session() = default;

    ~Session() = default;
private:
    std::shared_ptr <SessionImpl> impl_;
};
} // namespace HttpCli

#endif //TEST_SESSION_H
