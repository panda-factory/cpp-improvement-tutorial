//
// Created by admin on 2021/1/10.
//

#include "response.h"

namespace http {
ResponseHandler::ResponseHandler(std::function<void(Response&&)> handler) : handler_(handler) {

}

Response::Response(const std::int32_t &status_code, Error error, const std::string &body, const Header &header,
                   const Url &url, Cookies &&cookies)
        : status_code{status_code},
          error_(error),
          body_(body),
          header_(header),
          url_(url),
          cookies_(cookies) {

}

const std::string& Response::body() {
    return body_;
}

const Error& Response::error() {
    return error_;
}

void Response::SetBody(const std::string&& body) {
    body_ = std::move(body);
}

void ResponseHandler::operator()(Response&& response) {
    if (handler_ != nullptr) {
        handler_(std::forward<Response>(response));
    }
}

} // namespace http