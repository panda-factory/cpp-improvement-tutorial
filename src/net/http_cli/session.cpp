//
// Created by admin on 2021/1/10.
//

#include "session.h"

#include "core/macros.h"

#if (USE_HTTP_BEAST)
#include "implement/beast/session_impl_beast.h"
#else
#include "implement/ev/session_impl_ev.h"
#endif

namespace HttpCli {
Session::Session() {
#if (USE_HTTP_BEAST)
    impl_ = std::make_shared<SessionImplBeast>();
#else
    impl_ = std::make_shared<SessionImplEV>();
#endif
}

bool Session::Init() {
    return impl_->DoInit();
}

void Session::Request() {
    impl_->DoRequest();
}
} // namespace HttpCli
