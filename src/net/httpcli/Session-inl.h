//
// Created by admin on 2021/1/10.
//

#ifndef TEST_SESSION_INL_H
#define TEST_SESSION_INL_H

#include "Session.h"

namespace HttpCli {

inline void Session::SetUrl(const Url &url) {
    impl_->SetUrl(url);
}

inline void Session::SetParameters(const Parameters &parameters) {
    impl_->SetParameters(parameters);
}

inline void Session::SetParameters(Parameters &&parameters) {
    impl_->SetParameters(parameters);
}

inline void Session::SetHeader(const Header &header) {
    impl_->SetHeader(header);
}

inline void Session::SetTimeout(const Timeout &timeout) {
    impl_->SetTimeout(timeout);
}

inline void Session::SetAuth(const Authentication &auth) {
    impl_->SetAuth(auth);
}

inline void Session::SetDigest(const Digest &auth) {
    impl_->SetDigest(auth);
}

inline void Session::SetMultipart(Multipart &&multipart) {
    impl_->SetMultipart(multipart);
}

inline void Session::SetMultipart(const Multipart &multipart) {
    impl_->SetMultipart(multipart);
}

inline void Session::SetRedirect(const bool &redirect) {
    impl_->SetRedirect(redirect);
}

inline void Session::SetMaxRedirects(const MaxRedirects &max_redirects) {
    impl_->SetMaxRedirects(max_redirects);
}

inline void Session::SetCookies(const Cookies &cookies) {
    impl_->SetCookies(cookies);
}

inline void Session::SetBody(Body &&body) {
    impl_->SetBody(body);
}

inline void Session::SetBody(const Body &body) {
    impl_->SetBody(body);
}

// Used in templated functions
inline void Session::SetOption(const Url &url) {
    SetUrl(url);
}

inline void Session::SetOption(const Parameters &parameters) {
    SetParameters(parameters);
}

inline void Session::SetOption(Parameters &&parameters) {
    SetParameters(parameters);
}

inline void Session::SetOption(const Header &header) {
    SetHeader(header);
}

inline void Session::SetOption(const Timeout &timeout) {
    SetTimeout(timeout);
}

inline void Session::SetOption(const Authentication &auth) {
    SetAuth(auth);
}

inline void Session::SetOption(const Digest &digest) {
    SetDigest(digest);
}

inline void Session::SetOption(Multipart &&multipart) {
    SetMultipart(multipart);
}

inline void Session::SetOption(const Multipart &multipart) {
    SetMultipart(multipart);
}

inline void Session::SetOption(const bool &redirect) {
    SetRedirect(redirect);
}

inline void Session::SetOption(const MaxRedirects &maxRedirects) {
    SetMaxRedirects(maxRedirects);
}

inline void Session::SetOption(const Cookies &cookies) {
    SetCookies(cookies);
}

inline void Session::SetOption(Body &&body) {
    SetBody(body);
}

inline void Session::SetOption(const Body &body) {
    SetBody(body);
}

template<class H>
inline void Session::SetOption(const Handler <H> &&on_response) {
}
} // namespace HttpCli
#endif //TEST_SESSION_INL_H
