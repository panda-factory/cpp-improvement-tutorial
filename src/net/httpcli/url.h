//
// Created by admin on 2021/1/10.
//

#ifndef TEST_URL_H
#define TEST_URL_H
#include <string>
namespace HttpCli {
struct Url {
    std::string url;
    //!
    std::string protocol;

    //!
    std::string host;

    //!
    std::string port;

    //!
    std::string path;

    //!
    std::string parameters;

    //!
    std::string fragment;

    //! Wether the url requires TLS
    bool https() const {
        return protocol == "https";
    }

    Url(const std::string& url);
};
} //namespace HttpCli

#endif //TEST_URL_H
