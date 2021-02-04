//
// Created by admin on 2021/2/4.
//

#ifndef TEST_WEB_SOCKET_CLI_SESSION_IMPL_EV_H
#define TEST_WEB_SOCKET_CLI_SESSION_IMPL_EV_H

#include "net/ws_cli/session_impl.h"

namespace ws {
class SessionImplEV : public SessionImpl {
public:
    void DoConnect(const std::string& server, int port, const std::string& uri) override;
};

} // namespace ws


#endif //TEST_WEB_SOCKET_CLI_SESSION_IMPL_EV_H
