//
// Created by admin on 2021/2/6.
//

#ifndef TEST_FMT_STRING_H
#define TEST_FMT_STRING_H
#include <string>
namespace wtf {
template<typename ...Args>
std::string FmtString(const std::string& format, Args... args) {
    constexpr size_t oldlen = BUFSIZ;
    char buffer[oldlen]; // 默认栈上的缓冲区

    size_t newlen = snprintf(&buffer[0], oldlen, format.c_str(), args...);
    newlen++; // 算上终止符'\0'

    if (newlen > oldlen) { // 默认缓冲区不够大，从堆上分配
        std::vector<char> newbuffer(newlen);
        snprintf(newbuffer.data(), newlen, format.c_str(), args...);
        return std::string(newbuffer.data());
    }

    return buffer;
}
} // namespace wtf
#endif //TEST_FMT_STRING_H
