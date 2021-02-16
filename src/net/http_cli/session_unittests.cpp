//
// Created by admin on 2021/1/17.
//

#include "gtest/gtest.h"
#include "session.h"
#include "core/logging.h"

TEST(SessionTest, Session_GET_HTTP_BAIDU) {
    using namespace http;
    Session session;
    session.Init();
    Error error;
    session.Get(
            Url("http://www.baidu.com"),
            ResponseHandler([&](auto&& resp) {
                error = resp.error();
            }));
    ASSERT_EQ(error, ErrorCode::OK);
}

TEST(SessionTest, Session_GET_HTTPS_BAIDU) {
    using namespace http;
    Session session;
    session.Init();
    Error error;
    session.Get(
            Url("https://www.baidu.com"),
            ResponseHandler([&](auto&& resp) {
                error = resp.error();
            }));
    ASSERT_EQ(error, ErrorCode::OK);
}

TEST(SessionTest, Session_GET_0) {
    using namespace http;
    Session session;
    session.Init();
    Error error;
    session.Get(
            Url("http://httpbin.org/anything"),
            ResponseHandler([&](auto&& resp) {
                error = resp.error();
            }));
    ASSERT_EQ(error, ErrorCode::OK);
}

TEST(SessionTest, Session_GET_1) {
    using namespace http;
    Session session;
    session.Init();
    Error error;
    session.Get(
            Url("http://www.so.com/status.html"),
            ResponseHandler([&](auto&& resp) {
                error = resp.error();
            }));
    ASSERT_EQ(error, ErrorCode::OK);
}

TEST(SessionTest, Session_GET_2) {
    using namespace http;
    Session session;
    session.Init();
    Error error;
    session.Get(
            Url("http://www.360.cn/robots.txt"),
            ResponseHandler([&](auto&& resp) {
                error = resp.error();
            }));
    ASSERT_EQ(error, ErrorCode::OK);
}