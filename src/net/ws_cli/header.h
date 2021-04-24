//
// Created by admin on 2021/2/6.
//

#ifndef TEST_WS_CLI_HEADER_H
#define TEST_WS_CLI_HEADER_H

#include <cctype>
#include <map>

namespace net {
namespace ws {
struct CaseInsensitiveCompare {
    bool operator()(const std::string& a, const std::string& b) const noexcept {
        return std::lexicographical_compare(
                a.begin(), a.end(), b.begin(), b.end(),
                [](unsigned char ac, unsigned char bc) { return std::tolower(ac) < std::tolower(bc); });
    }
};
using Header = std::map<std::string, std::string, CaseInsensitiveCompare>;

} //namespace ws
} // namespace net

#endif //TEST_WS_CLI_HEADER_H
