//
// Created by admin on 2021-04-24.
//

#include <iostream>
#include <string>
#include <set>

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

    void SetOption(const A a) {
        SetA(a);
    }

    void SetOption(const B b) {
        SetB(b);
    }

    void SetOption(const C c) {
        SetC(c);
    }

#if CPP_11
    template<typename T>
    void SetOption(T&& t);

    template<typename T, typename... Ts>
    void SetOption(T&& t, Ts&&... ts);
#endif

    template<typename... Ts>
    void SetOption(Ts&&... ts);

private:
    A a_;
    B b_;
    C c_;
};
#if CPP_11
template<typename T>
void Object::SetOption(T&& t) {
    SetOption(std::forward<T>(t));
}

template<typename T, typename... Ts>
void Object::SetOption(T&& t, Ts&&... ts) {
    SetOption(std::forward<T>(t));
    SetOption(std::forward<Ts>(ts)...);
}
#endif

template<typename... Ts>
void Object::SetOption(Ts&&... ts) {
    (SetOption(std::forward<Ts>(ts)), ...);
}

int main() {
    Object obj;
    obj.SetA(A());
    obj.SetB(B());
    obj.SetC(C());

    obj.SetOption(A());
    obj.SetOption(B());
    obj.SetOption(C());

    obj.SetOption(A(), B(), C());
    return 0;
}
