//
// Created by admin on 2021/1/10.
//

#include "cookies.h"

#include <initializer_list>
#include <map>
#include <sstream>
#include <string>
#include <regex>
#include <utility>
#include <functional>
#include <iostream>

#include <boost/algorithm/string/trim_all.hpp>

namespace HttpCli {

void Cookies::ParseCookieString(const std::string& cookie_str) {
    using namespace boost::algorithm;

    std::regex cookies_rx(";");
    std::vector<std::string> cookies(
            std::sregex_token_iterator(
                    cookie_str.begin(), cookie_str.end(), cookies_rx, -1),
            std::sregex_token_iterator()
    );

    for (auto& cookie : cookies) {
        std::regex rx_cookie("([^=]+)=([^;]+)");
        std::smatch match;
        std::regex_match(cookie, match, rx_cookie);
        map_.insert(std::make_pair(trim_all_copy(match[1].str()), match[2].str()));
    }
}
} //namespace HttpCli