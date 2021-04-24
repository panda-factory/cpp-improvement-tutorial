//
// Created by admin on 2021/1/10.
//

#ifndef TEST_URL_H
#define TEST_URL_H
#include <string>

#include "core/macros.h"

namespace net {
struct Url {

    //! Wether the url requires TLS
    bool IsHttps() const;

    std::string url;
    //!
    std::string schema;

    //!
    std::string host;

    //!
    int port;

    //!
    std::string path;

    //!
    std::string query;

    //!
    std::string fragment;

    Url(const std::string &url);

    Url() = default;

private:
    void ParseUrl();

    //WTF_DISALLOW_COPY_AND_ASSIGN(Url);
};
} // namespace net

#endif //TEST_URL_H
