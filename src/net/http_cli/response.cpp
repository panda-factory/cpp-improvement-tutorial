//
// Created by admin on 2021/1/10.
//

#include "response.h"

namespace net {
namespace http {
ResponseHandler::ResponseHandler(std::function<void(Response&&)> handler) : handler_(handler) {

}

Response::Response(const std::int32_t &status_code, Error error, const std::string &body, const Header &header,
                   const Url &url, Cookies &&cookies)
        : statusCode{status_code},
          error(error),
          body(body),
          header(header),
          url(url),
          cookies(cookies) {

}


void ResponseHandler::operator()(Response&& response) {
    if (handler_ != nullptr) {
        handler_(std::forward<Response>(response));
    }
}

} // namespace http
} // namespace net