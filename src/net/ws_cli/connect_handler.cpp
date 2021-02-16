//
// Created by admin on 2021/2/15.
//

#include "connect_handler.h"

namespace ws {
ConnectHandler::ConnectHandler(std::function<void()> handler)
    : handler_(handler) {

}

void ConnectHandler::operator()() {
    if (handler_ != nullptr) {
        handler_();
    }
}

} // namespace ws