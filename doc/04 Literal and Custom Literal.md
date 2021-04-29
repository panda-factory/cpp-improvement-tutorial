---
title: Literal and Custom Literal
type: book-zh-cn
order: 4
---

# 第4章 字面量与自定义字面量

## 4.1 字面量
C++中我们所看到的常量是一种固定值，在程序执行期间不会改变。这些固定的值，又叫做字面量，即字面上看上去就知道的常量。比如Windows上的文件路径：C:\File\To\Path，而在编码中为了转义反斜杠'\'，我们不得已要将字符串定义成“C:\\File\\To\\Path”。另外有经验的开发者则会使用R字符串原始字面量，表示括号()中的内容极为最终的输出内容。
```C++
#include <iostream>
#include <string>

int main() {
    std::string path = R"(C:\File\To\Path)";
    std::cout << "path=" << path << std::endl;
    return 0;
}
```
在开发中，经常会遇到需要使用这样的使用字面量来表示某个常量，比如1000，是表示1000ms还是1000s。早在C++11就引进了自定义字面量的能力。

## 4.2 自定义字面量
自定义字面量的使用其实很简单，只要通过重载双引号后缀运算符实现，且当前只支持四种自定义字面量：
### 4.2.1 字符串字面量
- 必须使用 (const char *, size_t) 形式的参数表；
- 例：我想输出一个文件名，希望可以自动带上后缀名.exe：
```C++
std::string operator"" _exe (const char *file, size_t) {
    return file + std::string(".exe");
}

int main() {
    std::cout << "file=" << "panda"_exe << std::endl;
    return 0;
}
```
### 4.2.2 字符字面量
- 形参只能是 char, wchar_t, char16_t, char32_t 这几种类型；
- 例：我想将字符转大写：

```C++
char operator"" _xX (const char letter) {
    return toupper(letter);
}

int main() {
    std::cout << "to upper=" << 'a'_xX << std::endl;
    return 0;
}
```
### 4.2.3整型字面量
- 形参只能是unsigned long long、const char *;
- 例：用毫秒表示秒：
```C++
unsigned long long operator"" _s (unsigned long long s) {
    return s * 1000;
}

int main() {
    std::cout << "2s = " << 2_s << "mm" << std::endl;
    return 0;
}
```

### 4.2.4浮点型字面量
- 重载时必须使用 long double、const char *；
- 例：打印单位为kg的重量

```C++
std::string operator"" _kg (long double num) {
    return std::to_string(num) + "kg";
}

int main() {
    std::cout << "weight = " << 2.2_kg << std::endl;
    return 0;
}
```

更多精彩，请持续关注。
