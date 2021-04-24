//
// Created by admin on 2021/2/22.
//

#ifndef TEST_REQUEST_H
#define TEST_REQUEST_H

#include <sstream>
#include "header.h"
#include "method.h"
#include "url.h"


namespace net {
struct Request {
    void PreparePayload();

    Header header;
    Method method;
    Url url;
    unsigned short httpMajor = 1;
    unsigned short httpMinor = 1;
    std::string raw;
};
} // namespace net

#endif //TEST_REQUEST_H
