//
// Created by admin on 2021/1/10.
//

#include "session.h"

#include "wtf/macros.h"

#if (__USE_BEAST__)
#include "implement/beast/session_impl_beast.h"
#elif (__USE_LIBEVENT__)
#include "implement/ev/session_impl_ev.h"
#elif (__USE_LIBUV__)
#include "implement/uv/session_impl_uv.h"
#endif

namespace net {
namespace http {
Session::Session() {
#if USE_HTTP_BEAST
    impl_ = std::make_shared<SessionImplBeast>();
#elif __USE_LIBEVENT__
    impl_ = std::make_shared<SessionImplEV>();
#elif __USE_LIBUV__
    impl_ = std::make_shared<SessionImplUV>();
#endif
}

int Session::Init() {
    return impl_->DoInit();
}

void Session::Request() {
    impl_->DoRequest();
}
} // namespace http
} // namespace net
