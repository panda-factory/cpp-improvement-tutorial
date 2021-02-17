//
// Created by admin on 2021/2/16.
//

#ifndef TEST_HANDLER_H
#define TEST_HANDLER_H

namespace ws {
template<typename F>
class Handler {
public:
    void operator()();

    explicit Handler(std::function<T> handler);

    Handler() = default;

private:
    std::function<T> handler_ = nullptr;
};
} // namespace ws

#endif //TEST_HANDLER_H
