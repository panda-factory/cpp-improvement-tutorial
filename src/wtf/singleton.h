//
// Created by admin on 2021-07-03.
//

#ifndef TEST_SINGLETON_H
#define TEST_SINGLETON_H

namespace wtf {
template<typename T>
class Singleton {
public:
    static T* GetInstance() {
        static T t;
        return &t;
    }

    Singleton(const Singleton &) = delete;

    Singleton &operator=(const Singleton &) = delete;

protected:
    Singleton() = default;

    ~Singleton() = default;
};
} // namespace wtf
#endif //TEST_SINGLETON_H
