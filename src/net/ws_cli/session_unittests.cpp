//
// Created by admin on 2021/2/4.
//

#include "gtest/gtest.h"
#include "session.h"
#include "core/logging.h"

TEST(SessionTest, Session_WS) {
    using namespace ws;
    Session session;
    session.Init();
    session.SetOption(ConnectHandler([&] (){
        char *msg = strdup("hello");
        session.SendMsg(msg);
        free(msg);
    }));
    session.Connect("123.207.136.134", 9010, "ajaxchattest");
//    Error error;
    ASSERT_EQ(1, 1);
}
