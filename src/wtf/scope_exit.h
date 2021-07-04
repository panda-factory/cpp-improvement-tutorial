//
// Created by admin on 2021/1/17.
//

#ifndef TEST_SCOPE_EXIT_H
#define TEST_SCOPE_EXIT_H
#include <functional>
class ScopeExit {
public:
    ScopeExit() = default;
    ScopeExit(const ScopeExit&) = delete;
    void operator=(const ScopeExit&) = delete;

    template <typename F, typename... Args>
    ScopeExit(F&& f, Args&&... args) {
        func_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    }

    ~ScopeExit() {
        if (func_) {
            func_();
        }
    }

private:
    std::function<void()> func_;
};

#define __CONCAT(a, b) a##b
#define _MAKE_SCOPE_(line) ScopeExit __CONCAT(scope, line) = [&]()
#undef SCOPE_GUARD
#define SCOPE_GUARD _MAKE_SCOPE_(__LINE__)

#endif //TEST_SCOPE_EXIT_H
