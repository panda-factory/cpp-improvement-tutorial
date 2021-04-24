//
// Created by admin on 2021-03-07.
//

#ifndef TEST_WS_CLI_RESPONSE_H
#define TEST_WS_CLI_RESPONSE_H

#include <cstdint>
#include <string>

#include "net/url.h"
#include "net/header.h"

namespace net {
namespace ws {
struct Response {

    Response() = default;

    Response(const std::int32_t &status_code, const std::string &body, const Header &header,
             const Url &url);

    //! HTTP Status Code as Specified in [HTTP RFC](https://tools.ietf.org/html/rfc7231#section-6.1).
    std::int32_t statusCode;

    std::string status;

    //! Response body
    std::string body;

    //! Headers entries sent in the response
    Header header;

    //! Url that served this
    Url url;
};

} // namespace ws
} // namespace net

#endif //TEST_WS_CLI_RESPONSE_H
