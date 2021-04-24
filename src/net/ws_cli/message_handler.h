//
// Created by admin on 2021/2/16.
//

#ifndef TEST_MESSAGE_HANDLER_H
#define TEST_MESSAGE_HANDLER_H


#include <functional>

namespace net {
namespace ws {
class MessageHandler {
public:
    bool IsValid();
    void operator()(char *msg, uint64_t len, int binary, void *arg);

    explicit MessageHandler(std::function<void(char *msg, uint64_t len, int binary, void *arg)> handler);

    MessageHandler() = default;

private:
    bool isValid_ = false;
    std::function<void(char *msg, uint64_t len, int binary, void *arg)> handler_ = nullptr;
};
} // namespace ws
} // namespace net

#endif //TEST_MESSAGE_HANDLER_H
