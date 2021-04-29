---
title: Fold Expression
type: book-zh-cn
order: 5
---

# 第5章 折叠表达式
*  可变参数模板、折叠表达式的妙用；
*  对于概念方面的不想多讲，只需要在网上动动手，就能找到概念；
*  熊猫更希望以目的为导向，先知道为什么需要用，再去了解是什么；
## 5.1 关于封装
在绝大部分面向对象的开发中，业务往往是以功能模块划分的，尤其是现代C++编程。码友经常会遇到需要将功能模块封装，对外暴露接口的场景。比如有个类Object，拥有私有属性A、B、C，同时需要提供Set接口为属性赋值：
### 版本0
常规做法就是直接提供SetA、SetB、SetC等接口，用户使用时分别调用。
```C++
class A {
};

class B {
};

class C  {
};

class Object {
public:
    void SetA(A a) {
        a_ = a;
        std::cout << "SetA." << std::endl;
    }

    void SetB(B b) {
        b_ = b;
        std::cout << "SetB." << std::endl;
    }

    void SetC(C c) {
        c_ = c;
        std::cout << "SetC." << std::endl;
    }

private:
    A a_;
    B b_;
    C c_;
};

int main() {
    Object obj;
    obj.SetA(A());
    obj.SetB(B());
    obj.SetC(C());
    return 0;
}
```
这个版本对封装者来说方便，但对用户麻烦，且观感不佳。互联网的常识是：专业的人干专业的事，所以怎么能把麻烦留给用户。

### 版本1
有经验的码友会想到重载函数，提供统一接口SetOption给用户使用。
```C++
......
class Object {
public:
    ......

    void SetOption(const A a) {
        SetA(a);
    }

    void SetOption(const B b) {
        SetB(b);
    }

    void SetOption(const C c) {
        SetC(c);
    }
    ......
};

int main() {
    Object obj;

    obj.SetOption(A());
    obj.SetOption(B());
    obj.SetOption(C());
    return 0;
}
```
这个版本虽然开发者需要多谢几个接口，但对用户来说，只需记住一个接口，分别传不同参数即可。但仔细再一看，除了参数不同外，用户必须连续写多行基本一致的代码。还是不够友好、通用。

## 5.2 可变模板参数的表用
C++11的新特性——可变模版参数（variadic templates），是C++11新增的最强大的特性之一，它对参数进行了高度泛化。废话不多说，直接看版本2：
### 版本2
```C++

class Object {
public:
    ......

    template<typename T>
    void SetOption(T&& t);

    template<typename T, typename... Ts>
    void SetOption(T&& t, Ts&&... ts);

    ......
};

template<typename T>
void Object::SetOption(T&& t) {
    SetOption(std::forward<T>(t));
}

template<typename T, typename... Ts>
void Object::SetOption(T&& t, Ts&&... ts) {
    SetOption(std::forward<T>(t));
    SetOption(std::forward<Ts>(ts)...);
}

int main() {
    Object obj;

    obj.SetOption(A(), B(), C());
    return 0;
}
```
这里使用了两个模板函数：
*  SetOption(T&& t, Ts&&... ts)：用于接收可变参数，并解出出首个参数T t，及其余参数包Ts... ts；
*  SetOption(T&& t)：用于接收单个参数的情况，必须要定义，不然只有单个参数时，会进入无线递归。（在本例中不会，因为本例已经为单个参数的情况做了定义）；

跟版本1对比，版本2对用户更加友好了，只需使用一行代码，就可同时为对象Set多个属性。但是（又反转了，真烦），对开发者来说，确实不够友好。因为在使用C++11的可变模板参数时，为了拆解参数包，往往都要多定义一个单参数的模板，用来结束递归。

## 5.3 折叠表达式
*  名为：Fold Expression；
*  C++17才支持的特性，需要开启stdc++17编译选项的支持；

好吧，这篇的标题就是这个，所以主要就是为了介绍折叠表达式，先看代码吧，爆赞：
### 版本3
```C++
class Object {
public:
    ......

    template<typename... Ts>
    void SetOption(Ts&&... ts);

    ......
};

template<typename... Ts>
void Object::SetOption(Ts&&... ts) {
    (SetOption(std::forward<Ts>(ts)), ...);
}

int main() {

    obj.SetOption(A(), B(), C());
    return 0;
}
```
版本3对用户来说体验还是一样优秀，对开发者而言，舒适度提高不少。这里使用了一元右折叠表达式对可变参数包进行拆解，感兴趣的也可以换成左折叠，对本篇代码来说，效果一样。

### 你学废了嘛？

更多精彩，请持续关注。。。
