//
// Created by admin on 2021/2/22.
//

#include "request.h"
#include "core/fmt_string.h"

namespace http {
void Request::PreparePayload() {
    raw = wtf::FmtString("%s %s HTTP/%d.%d\r\n",
                          method.c_str(),
                          url.path.empty() ? "/" : url.path.c_str(),
                          httpMajor, httpMinor);
    raw += wtf::FmtString("Host: %s\r\n",
                          url.host.c_str());



    raw += "\r\n";
}
} // namespace http