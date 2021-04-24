//
// Created by admin on 2021/1/9.
//
#include <iostream>
#include "net/http_cli/session.h"
#include "net/ws_cli/session.h"
#include "net/url.h"
int main() {
    using namespace net;
    using namespace net::http;
    Session session;
    session.Init();
    session.Get(
            Url("http://www.baidu.com"));
    return 0;
}