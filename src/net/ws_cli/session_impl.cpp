//
// Created by admin on 2021/2/3.
//

#include "session_impl.h"
namespace net {
namespace ws {

void SessionImpl::SetOption(const ConnectHandler&& connect_handler) {
    SetHandler(std::move(connect_handler));
}
void SessionImpl::SetOption(const MessageHandler&& message_handler) {
    SetHandler(std::move(message_handler));
}
void SessionImpl::SetOption(const Url &url) {
    SetUrl(url);
}
} // namespace ws
} // namespace net