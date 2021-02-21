//
// Created by admin on 2021/1/10.
//

#ifndef TEST_SESSION_H
#define TEST_SESSION_H

#include "net/http_cli/session_impl.h"

namespace http {

class Session {
public:
    int Init();

    template<typename... Ts>
    void Get(Ts&&... ts);

    template<typename T>
    void SetOption(T&& t);

    template<typename T, typename... Ts>
    void SetOption(T&& t, Ts&&... ts);

    void Request();

    Session();

    ~Session() = default;
private:
    std::shared_ptr <SessionImpl> impl_;
};

template<typename... Ts>
void Session::Get(Ts&&... ts) {
    SetOption(std::forward<Ts>(ts)...);
    impl_->DoRequest();
}

template<typename T>
void Session::SetOption(T&& t) {
    impl_->SetOption(std::forward<T>(t));
}

template<typename T, typename... Ts>
void Session::SetOption(T&& t, Ts&&... ts) {
    SetOption(std::forward<T>(t));
    SetOption(std::forward<Ts>(ts)...);
}


} // namespace http

#endif //TEST_SESSION_H
