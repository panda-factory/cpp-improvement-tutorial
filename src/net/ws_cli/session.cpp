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

int Session::Close() {
    return impl_->DoClose();
}

int Session::Connect(const std::string& server, int port, const std::string& uri) {
    return impl_->DoConnect(server, port, uri);
}
bool Session::Init() {
    return impl_->DoInit();
}

int Session::SendMsg(const std::string& msg) {
    return impl_->DoSendMsg(msg);
}
} // namespace ws