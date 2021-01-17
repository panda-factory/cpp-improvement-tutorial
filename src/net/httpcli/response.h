//
// Created by admin on 2021/1/10.
//

#ifndef TEST_RESPONSE_H
#define TEST_RESPONSE_H

#include "core/macros.h"

#include "net/httpcli/cookies.h"
#include "net/httpcli/error.h"
#include "net/httpcli/Header.h"
#include "net/httpcli/url.h"

namespace HttpCli {
/**
 * \brief Passed to the unary callback xxhr::on_response.
 *        Provides access to request Content and Success, HTTP Status Codes or Errors.
 */
class Response {
public:
    Response() = default;

    template<typename TextType, typename HeaderType, typename UrlType, typename CookiesType,
            typename ErrorType>
    Response(const std::int32_t &p_status_code, ErrorType &&p_error, TextType &&p_text, HeaderType &&p_header,
             UrlType &&p_url, CookiesType &&p_cookies = Cookies{})
            : status_code{p_status_code},
              error{CORE_FWD(p_error)},
              text{CORE_FWD(p_text)},
              header{CORE_FWD(p_header)},
              url{CORE_FWD(p_url)},
              cookies{CORE_FWD(p_cookies)} {}

    //! HTTP Status Code as Specified in [HTTP RFC](https://tools.ietf.org/html/rfc7231#section-6.1).
    std::int32_t status_code;

    //! Error condition proper to a lower layer than HTTP.
    Error error;

    //! Response body
    std::string text;

    //! Headers entries sent in the response
    Header header;

    //! Url that served this
    Url url;

    //! Cookies that the server would like you to keep reminding in future queries.
    Cookies cookies;

};
} // namespace HttpCli


#endif //TEST_RESPONSE_H
