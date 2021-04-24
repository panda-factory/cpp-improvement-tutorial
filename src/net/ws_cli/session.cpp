//
// Created by admin on 2021/2/3.
//

#include "session.h"

#if USE_HTTP_BEAST
#include "implement/beast/session_impl_beast.h"
#elif __USE_LIBEVENT__
#include "implement/ev/session_impl_ev.h"
#elif __USE_LIBUV__
#include "implement/uv/session_impl_uv.h"
#endif

namespace net {
namespace ws {

Session::Session() {
#if USE_HTTP_BEAST
    impl_ = std::make_shared<SessionImplBeast>();
#elif __USE_LIBEVENT__
    impl_ = std::make_shared<SessionImplEV>();
#elif __USE_LIBUV__
    impl_ = std::make_shared<SessionImplUV>();
#endif
}

int Session::Close() {
    return impl_->DoClose();
}

int Session::Connect() {
    return impl_->DoConnect();
}
bool Session::Init() {
    return impl_->DoInit();
}

int Session::SendMsg(const std::string& msg) {
    return impl_->DoSendMsg(msg);
}
} // namespace ws
} // namespace net