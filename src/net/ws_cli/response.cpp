//
// Created by admin on 2021-03-07.
//

#include "response.h"

namespace net {
namespace ws {

Response::Response(const std::int32_t &status_code, const std::string &body, const Header &header,
                   const Url &url)
        : statusCode{status_code},
          body(body),
          header(header),
          url(url){

}
} // namespace ws
} // namespace net
