﻿//
// Created by admin on 2021/1/9.
//
#include <iostream>
#include "net/http_cli/session.h"

int main() {
    using namespace http;
    Session session;
    session.Init();
    session.Get(
            Url("http://www.baidu.com"));
    return 0;
}