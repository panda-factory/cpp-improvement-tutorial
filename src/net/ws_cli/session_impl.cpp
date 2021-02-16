//
// Created by admin on 2021/2/3.
//

#include "session_impl.h"
namespace ws {

void SessionImpl::SetOption(const ConnectHandler& onConnect) {
    SetHandler(onConnect);
}
void SessionImpl::SetOption(const MessageHandler& onMessage) {
    SetHandler(onMessage);
}
} // namespace ws