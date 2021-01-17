//
// Created by admin on 2021/1/10.
//

#ifndef TEST_HEADER_H
#define TEST_HEADER_H

#include <cctype>
#include <map>


namespace HttpCli {
struct CaseInsensitiveCompare {
    bool operator()(const std::string& a, const std::string& b) const noexcept {
        return std::lexicographical_compare(
                a.begin(), a.end(), b.begin(), b.end(),
                [](unsigned char ac, unsigned char bc) { return std::tolower(ac) < std::tolower(bc); });
    }
};
using Header = std::map<std::string, std::string, CaseInsensitiveCompare>;

} //namespace HttpCli


#endif //TEST_HEADER_H
