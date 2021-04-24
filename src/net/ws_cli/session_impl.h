//
// Created by admin on 2021/2/3.
//

#ifndef TEST_WEB_SOCKET_CLI_SESSION_IMPL_H
#define TEST_WEB_SOCKET_CLI_SESSION_IMPL_H
#include <string>

#include "connect_handler.h"
#include "message_handler.h"
#include "net/method.h"
#include "net/url.h"
#include "net/request.h"

namespace net {
namespace ws {

class SessionImpl {
public:
    virtual int DoClose() = 0;
    virtual int DoInit() = 0;
    virtual int DoConnect() = 0;
    virtual int DoSendMsg(const std::string& msg) = 0;

    virtual void SetHandler(const ConnectHandler&& connect_handler) = 0;
    virtual void SetHandler(const MessageHandler&& message_handler) = 0;
    virtual void SetUrl(const Url& url) = 0;

    void SetOption(const ConnectHandler&& connect_handler);
    void SetOption(const MessageHandler&& message_handler);
    void SetOption(const Url& url);
};
} // namespace ws
} // namespace net


#endif //TEST_WEB_SOCKET_CLI_SESSION_IMPL_H
