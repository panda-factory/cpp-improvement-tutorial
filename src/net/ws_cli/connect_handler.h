//
// Created by admin on 2021/2/15.
//

#ifndef TEST_CONNECTHANDLER_H
#define TEST_CONNECTHANDLER_H

#include <functional>

namespace net {
namespace ws {
class ConnectHandler {
public:
    void operator()();

    explicit ConnectHandler(std::function<void()> handler);

    ConnectHandler() = default;

private:
    std::function<void()> handler_ = nullptr;
};
} // namespace ws
} // namespace net

#endif //TEST_CONNECTHANDLER_H
