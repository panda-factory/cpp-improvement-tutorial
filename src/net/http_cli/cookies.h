//
// Created by admin on 2021/1/10.
//

#ifndef TEST_COOKIES_H
#define TEST_COOKIES_H
#include <map>
#include <string>
namespace net {
namespace http {
struct Cookies {
public:
    void ParseCookieString(const std::string &cookie_str);

private:
    std::map<std::string, std::string> map_;
};
} // namespace http
} // namespace net

#endif //TEST_COOKIES_H
