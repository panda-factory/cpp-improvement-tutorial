//
// Created by admin on 2021/1/10.
//

#ifndef TEST_URL_H
#define TEST_URL_H
#include <string>

#include "core/macros.h"
namespace http::cli {
struct Url {

    //! Wether the url requires TLS
    bool IsHttps() const;

    Url(const std::string& url);

    Url() = default;

    std::string url;
    //!
    std::string protocol;

    //!
    std::string host;

    //!
    int port;

    //!
    std::string path;

    //!
    std::string parameters;

    //!
    std::string fragment;
private:
    void ParseUrl();

    //CORE_DISALLOW_COPY_AND_ASSIGN(Url);
};
} //namespace http::cli

#endif //TEST_URL_H
