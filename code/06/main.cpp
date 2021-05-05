//
// Created by admin on 2021-04-24.
//

#include <iostream>
#include <string>
#include <map>

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


int main() {
    Index range(0, 10);

    for (auto value : range) {
        std::cout << value << std::endl;
    }

    return 0;
}
