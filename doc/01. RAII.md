---
title: RAII
type: book-zh-cn
order: 1
---

# 第1章 RAII

[TOC]

## 1.1 RAII是什么

RAII (Resource Acquisition Is Initialization，资源获取即初始化)，将资源封装在类中，构造函数申请资源，析构函数释放资源。即将资源生命周期与类生命周期绑定。对于开发人员最忌讳的就是诸如内存泄漏等低级错误，而往往内存泄漏都是由于开发人员的粗心，在申请完资源后，忘记释放内存。而对象实例在生命周期结束时，会自动调用析构函数，将资源与类声明周期绑定，是面向对象编程的特有设计思想。在C++的STL基本都遵循RAII规范，std::string就是最直观的典型。

## 1.2 RAII妙用Scope Exit Do

这里要介绍的是RAII的另一种妙用，Scope Exit Do，即作用域结束时调用。用过C++对线程编程的肯定对std::mutex不会陌生，而往往令开发人员抓狂的除了lock()后要时刻记住unlock()，还有到处都有unlock()的代码。有代码洁癖的人简直想将屏幕打碎：
```
std::mutex mutex;
void DoSomething() {
    ...
}

void DoWithoutRAII {
    mutex.lock(); // 上锁，避免重入
    DoSomething();
    if (condition1) {
        ...
        mutex.unlock(); // 解锁 1
        return;
    } else if (condition2) {
        ...
        mutex.unlock(); // 解锁 2
        return;
    } else {
        ...
        mutex.unlock(); // 解锁 3
        return;
    }
    ...
    mutex.unlock(); // 解锁 4
    return;
}
```
上述代码有4处异常分支，而导致需要调用unlock共计4次。而C++早为我们提供了RAII的Scope Exit Do风格的std::lock_guard，简单来说，就是利用局部对象生命周期结束时，自动调用析构函数的特点，将mutex对象的unlock操作绑定在析构函数中。看下面代码：
```
std::mutex mutex;
void DoSomething() {
    ...
}

void DoWithRAII {
    std::lock_guard<std::mutex> lock(mutex); // Scope Exit风格
    DoSomething();
    if (condition1) {
        ...
        return;
    } else if (condition2) {
        ...
        return;
    } else {
        ...
        return;
    }
    ...
    return;
}
```
上面的代码中，并不是忘了调用unlock，而是将mutex与scope对象std::lock_guard绑定，当函数DoWithRAII结束时，局部变量lock被析构，则自动调用mutex.unlock()。

## 1.3 通用Scope Exit Do
boost利用了RAII特性，提供了很高级的ScopeExit封装，可以在作用域结束时自动调用绑定操作。直接看代码：
```
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

#define _CONCAT(a, b) a##b
#define _MAKE_SCOPE_(line) ScopeExit _CONCAT(scope, line) = [&]()
#undef SCOPE_GUARD
#define SCOPE_GUARD _MAKE_SCOPE_(__LINE__)
```
使用方法：
```
std::mutex mutex;
void DoSomething() {
    ...
}

void DoWithRAII {
    mutex.lock(); // Scope Exit风格
    SCOPE_GUARD {
        mutex.unlock();
    }
    DoSomething();
    if (condition1) {
        ...
        return;
    } else if (condition2) {
        ...
        return;
    } else {
        ...
        return;
    }
    ...
    return;
}
```

更多精彩，请持续关注。