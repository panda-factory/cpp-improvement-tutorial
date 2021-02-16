//
// Created by admin on 2021/2/16.
//

#include "message_handler.h"

namespace ws {
MessageHandler::MessageHandler(std::function<void(char *msg, uint64_t len, int binary, void *arg)> handler)
        : handler_(handler) {
    isValid_ = true;
}

bool MessageHandler::IsValid() {
    return isValid_;
}

void MessageHandler::operator()(char *msg, uint64_t len, int binary, void *arg) {
    if (handler_ != nullptr) {
        handler_(msg, len, binary, arg);
    }
}

} // namespace ws