---
title: Range For
type: book-zh-cn
order: 6
---

# 第6章 Range For
*  C++11开始支持基于范围的For循环，可参阅[Range-based for loop](https://en.cppreference.com/w/cpp/language/range-for)；
*  现代C++编程一种较为常见的特性，本章也会介绍如何自定义Range For循环；
*  C++的STL容器可以直接使用Range For，自定义类，需要实现begin()，end()才能使用Range For；
## 6.1 STL容器的Range For
主要是之前看到有热心网友找熊猫review代码，然后看到有如下代码的for循环：
```C++
    std::map<std::string, std::string> json {
        {"1", "1"},
        {"2", "2"},
        {"3", "3"},
        {"4", "4"}
    };

    auto begin = json.begin();
    auto end = json.end();
    for (; begin != end; begin++) {
        std::pair<std::string, std::string> value = *begin;
        if (value.first == "3") {
            std::cout << "find 3 key and value = " << value.second << std::endl;
        }
    }
```
说实话，这没毛病，但也没必要。C++虽然提供了为大部分容器都提供了迭代器，用于访问容器元素，但一般都不会直接拿出来使用，基本的使用场景，也是用于封装。直接使用迭代器，一方面影响代码可读性，另一方面不安全。那访问容器内部的元素，现代C++编程中更推荐使用Range For：
```C++
    ......
    for (const auto& value : json) {
        if (value.first == "3") {
            std::cout << "find 3 key and value = " << value.second << std::endl;
        }
    }
```
代码看起来明显清爽了不少。但注意，这中Range For不是想用就能用的，比如自定义类，是不是也能使用Range For遍历私有元素？
## 6.2 自定义类的Range For
自定义类要使用Range For需要满足以下几个条件：
*  begin()，end()：自定义类需要实现public域的begin()、end()方法，并返回迭代器；
*  迭代器必须支持三种操作：!=，前缀++，解引用。这个主要是因为Range For会被解释成：
```C++
{
    auto && __range = range_expression ;
    for (auto __begin = begin_expr, __end = end_expr; __begin != __end; ++__begin) {
        range_declaration = *__begin;
        loop_statement
    }
    // begin_expr = begin(); end_expr = end()
}
```
下面先看迭代器实现，如果需要访问的元素已经拥有迭代器，那就不必要自己实现迭代器。
#### 迭代器
```C++
template<typename T>
class Iter {
public:
    explicit Iter(T addr)
    : addr_(addr){}

    // 重载操作符：!=
    bool operator != (const Iter& that) const {
        return (this->addr_) != (that.addr_);
    }

    // 重载操作符：解引用
    T operator* () const {
        return addr_;
    }

    // 重载操作符：前缀++
    const Iter& operator++ () {
        ++addr_;
        return *this;
    }
private:
    T addr_; // 假设是元素地址
};
```
这个迭代器中存了容器的地址，并重载了Range For需要的三种操作。
下面我们自定义类Index，需要遍历下标元素，并将值打印出来：
#### 自定义类
```C++
class Index {
public:
    // begin_expr and end_expr
    Iter<int> begin () const {
        return Iter(begin_);
    }
    Iter<int> end () const {
        return Iter(end_ );
    }

    Index(int begin, int end) : begin_(begin), end_(end) {}
private:
    int begin_;
    int end_;
};
```
自定义类中，一定要实现begin表达式跟end表达式，并返回对应迭代器。
#### main.cpp
```C++
int main() {
    Index range(0, 10);

    for (auto value : range) {
        std::cout << value << std::endl;
    }

    return 0;
}
```
最后可以看到，在遍历时，会把下标的值挨个打印一遍
```
# bin\main.exe
0
1
2
3
4
5
6
7
8
9
```
### 你鞋废了嘛？

更多精彩，请持续关注。。。
