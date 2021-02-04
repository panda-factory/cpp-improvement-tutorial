//
// Created by admin on 2021/2/3.
//

#ifndef TEST_WEB_SOCKET_CLI_SESSION_IMPL_H
#define TEST_WEB_SOCKET_CLI_SESSION_IMPL_H
#include <string>
namespace ws {

class SessionImpl {
public:
    virtual void DoConnect(const std::string& server, int port, const std::string& uri) = 0;
};
} // namespace ws


#endif //TEST_WEB_SOCKET_CLI_SESSION_IMPL_H
