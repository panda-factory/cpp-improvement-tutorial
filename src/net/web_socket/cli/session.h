//
// Created by admin on 2021/2/3.
//

#ifndef TEST_WEB_SOCKET_CLI_SESSION_H
#define TEST_WEB_SOCKET_CLI_SESSION_H

namespace ws::cli {
class SessionImpl;
class Session {
public:
    bool Init();

    int Connect();

    Session();

    ~Session() = default;
private:
    std::shared_ptr <SessionImpl> impl_;
};
} //namespace ws::cli

#endif //TEST_WEB_SOCKET_CLI_SESSION_H
