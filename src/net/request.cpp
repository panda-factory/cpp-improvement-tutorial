//
// Created by admin on 2021/2/22.
//

#include "request.h"
#include "core/fmt_string.h"
#include "core/logging.h"

namespace net {
void Request::PreparePayload() {
    raw = wtf::FmtString("%s %s HTTP/%d.%d\r\n",
                          method.c_str(),
                          url.path.empty() ? "/" : url.path.c_str(),
                          httpMajor, httpMinor);
    raw += wtf::FmtString("Host: %s\r\n",
                          url.host.c_str());

    for (const auto& it : header) {
        raw += wtf::FmtString("%s: %s\r\n", it.first.c_str(), it.second.c_str());
    }

    raw += "\r\n";
    WTF_LOG(INFO) << raw;
}
} // namespace net