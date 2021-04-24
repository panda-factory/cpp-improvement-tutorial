//
// Created by admin on 2021/2/4.
//

#include "gtest/gtest.h"
#include "session.h"
#include "core/logging.h"

TEST(WSSessionTest, Hello) {
    using namespace net;
    using namespace net::ws;
    std::string msgRecv;
    Session session;
    session.Init();
    session.SetOption(
            Url("ws://123.207.136.134:9010/ajaxchattest"),
            ConnectHandler([&] () {
                session.SendMsg("hello");
            }),
            MessageHandler([&](char *msg, uint64_t len, int binary, void *arg) {
                msgRecv = msg;
                session.Close();
            }));
    session.Connect();
    ASSERT_EQ(msgRecv, "hello [<a href='http://coolaf.com/tool/chattest'>http://coolaf.com</a>]");
}
#if 0
TEST(WSSessionTest, Session_WS) {
    using namespace net;
    using namespace net::ws;
    std::string msgRecv;
    bool isFinish = false;
    Session session;
    session.Init();
    session.SetOption(
            Url("ws://123.207.136.134:9010/ajaxchattest"),
            ConnectHandler([&] () {
                session.SendMsg("hello");
            }),
            MessageHandler([&](char *msg, uint64_t len, int binary, void *arg) {
                if (!isFinish) {
                    session.SendMsg("Nice to see you again.");
                    isFinish = true;
                } else {
                    msgRecv = msg;
                    session.Close();
                }
            }));
    session.Connect();
    ASSERT_EQ(msgRecv, "Nice to see you again. [<a href='http://coolaf.com/tool/chattest'>http://coolaf.com</a>]");
}
#endif