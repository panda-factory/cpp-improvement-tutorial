//
// Created by admin on 2021/1/10.
//

#ifndef TEST_RESPONSE_H
#define TEST_RESPONSE_H

#include <functional>

#include "core/macros.h"
#include "net/http_cli/cookies.h"
#include "net/http_cli/error.h"
#include "net/http_cli/header.h"
#include "net/http_cli/url.h"

namespace HttpCli {
/**
 * \brief Passed to the unary callback xxhr::on_response.
 *        Provides access to request Content and Success, HTTP Status Codes or Errors.
 */
class Response {
public:
    const std::string& body();

    const Error& error();

    Response() = default;

    Response(const std::int32_t &status_code, Error error, const std::string& body, const Header& header,
             const Url& url, Cookies &&cookies = Cookies{});

private:
    //! HTTP Status Code as Specified in [HTTP RFC](https://tools.ietf.org/html/rfc7231#section-6.1).
    std::int32_t status_code;

    //! Error condition proper to a lower layer than HTTP.
    Error error_;

    //! Response body
    std::string body_;

    //! Headers entries sent in the response
    Header header_;

    //! Url that served this
    Url url_;

    //! Cookies that the server would like you to keep reminding in future queries.
    Cookies cookies_;

};
class ResponseHandler {
public:
    void operator()(Response&& response);

    explicit ResponseHandler(std::function<void(Response&&)> handler);
    ResponseHandler() = default;
private:
    std::function<void(Response&&)> handler_ = nullptr;
};

} // namespace HttpCli


#endif //TEST_RESPONSE_H
