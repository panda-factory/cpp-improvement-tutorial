//
// Created by admin on 2021/1/17.
//
#include "session_impl.h"

namespace http {
// Used in templated functions
void SessionImpl::SetOption(const Url &url) {
    SetUrl(url);
}

void SessionImpl::SetOption(const Parameters &parameters) {
    SetParameters(parameters);
}

void SessionImpl::SetOption(Parameters &&parameters) {
    SetParameters(parameters);
}

void SessionImpl::SetOption(const Header &header) {
    SetHeader(header);
}

void SessionImpl::SetOption(const Timeout &timeout) {
    SetTimeout(timeout);
}

void SessionImpl::SetOption(const authentication &auth) {
    SetAuth(auth);
}

void SessionImpl::SetOption(const Digest &digest) {
    SetDigest(digest);
}

void SessionImpl::SetOption(const Method& method) {
    SetMethod(method);
}

void SessionImpl::SetOption(Multipart &&multipart) {
    SetMultipart(multipart);
}

void SessionImpl::SetOption(const Multipart &multipart) {
    SetMultipart(multipart);
}

void SessionImpl::SetOption(const bool &redirect) {
    SetRedirect(redirect);
}

void SessionImpl::SetOption(const Retries&& retries) {
    SetRetries(retries);
}

void SessionImpl::SetOption(const MaxRedirects &maxRedirects) {
    SetMaxRedirects(maxRedirects);
}

void SessionImpl::SetOption(const Cookies &cookies) {
    SetCookies(cookies);
}

void SessionImpl::SetOption(body &&body) {
    SetBody(body);
}

void SessionImpl::SetOption(const body &body) {
    SetBody(body);
}

void SessionImpl::SetOption(const ResponseHandler&& onResponse) {
    SetHandler(std::move(onResponse));
}
} // namespace http