//
// Created by admin on 2021/1/17.
//

#ifndef TEST_MACROS_H
#define TEST_MACROS_H

#define CORE_FWD(...) ::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)
#endif //TEST_MACROS_H
