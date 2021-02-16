//
// Created by admin on 2021/2/3.
//

#ifndef TEST_WEB_SOCKET_CLI_SESSION_IMPL_H
#define TEST_WEB_SOCKET_CLI_SESSION_IMPL_H
#include <string>

#include "connect_handler.h"
#include "message_handler.h"

namespace ws {

class SessionImpl {
public:
    virtual int DoInit() = 0;
    virtual int DoConnect(const std::string& server, int port, const std::string& uri) = 0;
    virtual int DoSendMsg(char *msg) = 0;

    virtual void SetHandler(const ConnectHandler& onConnect) = 0;
    virtual void SetHandler(const MessageHandler& onMessage) = 0;

    void SetOption(const ConnectHandler& onConnect);
    void SetOption(const MessageHandler& onMessage);
};
} // namespace ws


#endif //TEST_WEB_SOCKET_CLI_SESSION_IMPL_H
