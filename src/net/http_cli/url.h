//
// Created by admin on 2021/1/10.
//

#ifndef TEST_URL_H
#define TEST_URL_H
#include <string>

#include "core/macros.h"
namespace http {
struct Url {

    //! Wether the url requires TLS
    bool IsHttps() const;

    Url(const std::string& url);

    Url() = default;

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
private:
    void ParseUrl();

    //WTF_DISALLOW_COPY_AND_ASSIGN(Url);
};
} //namespace http

#endif //TEST_URL_H
