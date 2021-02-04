//
// Created by admin on 2021/2/3.
//

#include "session.h"

#if (USE_HTTP_BEAST)
#include "implement/beast/session_impl_beast.h"
#else
#include "implement/ev/session_impl_ev.h"
#endif

namespace ws {
Session::Session() {
#if (USE_HTTP_BEAST)
    impl_ = std::make_shared<SessionImplBeast>();
#else
    impl_ = std::make_shared<SessionImplEV>();
#endif
}

int Session::Connect(const std::string& server, int port, const std::string& uri) {
    impl_->DoConnect(server, port, uri);
    return 0;
}
bool Session::Init() {
    return 0;
}
} // namespace ws