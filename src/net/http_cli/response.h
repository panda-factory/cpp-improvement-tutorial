//
// Created by admin on 2021/1/10.
//

#ifndef TEST_RESPONSE_H
#define TEST_RESPONSE_H

#include <functional>

#include "wtf/macros.h"
#include "net/http_cli/cookies.h"
#include "net/http_cli/error.h"
#include "net/header.h"
#include "net/url.h"

namespace net {
namespace http {

struct Response {
public:

    Response() = default;

    Response(const std::int32_t &status_code, Error error, const std::string &body, const Header &header,
             const Url &url, Cookies &&cookies = Cookies{});

    //! HTTP Status Code as Specified in [HTTP RFC](https://tools.ietf.org/html/rfc7231#section-6.1).
    std::int32_t statusCode;

    std::string status;

    //! Error condition proper to a lower layer than HTTP.
    Error error;

    //! Response body
    std::string body;

    //! Headers entries sent in the response
    Header header;

    //! Url that served this
    Url url;

    //! Cookies that the server would like you to keep reminding in future queries.
    Cookies cookies;

};

class ResponseHandler {
public:
    void operator()(Response &&response);

    explicit ResponseHandler(std::function<void(Response &&)> handler);

    ResponseHandler() = default;

private:
    std::function<void(Response &&)> handler_ = nullptr;
};

} // namespace http
} // namespace net

#endif //TEST_RESPONSE_H
