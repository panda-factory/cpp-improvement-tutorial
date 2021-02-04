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

    int Connect(const std::string& server, int port, const std::string& uri);

    Session();

    ~Session() = default;
private:
    std::shared_ptr <SessionImpl> impl_;
};
} //namespace ws

#endif //TEST_WEB_SOCKET_CLI_SESSION_H
