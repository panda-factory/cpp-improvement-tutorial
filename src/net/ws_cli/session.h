//
// Created by admin on 2021/2/3.
//

#ifndef TEST_WEB_SOCKET_CLI_SESSION_H
#define TEST_WEB_SOCKET_CLI_SESSION_H
#include <memory>
#include <string>

#include "session_impl.h"

namespace ws {
class Session {
public:
    bool Init();

    int Close();

    int Connect(const std::string& server, int port, const std::string& uri);

    int SendMsg(const std::string& msg);

    template<typename T>
    void SetOption(T&& t);

    template<typename T, typename... Ts>
    void SetOption(T&& t, Ts&&... ts);

    Session();

    ~Session() = default;
private:
    std::shared_ptr <SessionImpl> impl_;
};

template<typename T>
void Session::SetOption(T&& t) {
    impl_->SetOption(std::forward<T>(t));
}

template<typename T, typename... Ts>
void Session::SetOption(T&& t, Ts&&... ts) {
    SetOption(std::forward<T>(t));
    SetOption(std::forward<Ts>(ts)...);
}

} //namespace ws

#endif //TEST_WEB_SOCKET_CLI_SESSION_H
