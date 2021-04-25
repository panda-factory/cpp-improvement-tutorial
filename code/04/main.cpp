//
// Created by admin on 2021-04-24.
//

#include <iostream>
#include <string>

std::string operator"" _exe (const char *file, size_t) {
    return file + std::string(".exe");
}

std::string operator"" _xX (const char letter) {
    return std::string(".exe");
}

unsigned long long operator"" _s (unsigned long long s) {
    return s * 1000;
}

std::string operator"" _kg (long double num) {
    return std::to_string(num) + "kg";
}

int main() {
    std::string path = R"(C:\File\To\Path)";
    std::cout << "path=" << 1000 << std::endl;
    std::cout << "file=" << "panda"_exe << std::endl;
    std::cout << "to upper=" << 'a'_xX << std::endl;
    std::cout << "2s = " << 2_s << "mm" << std::endl;
    std::cout << "weight = " << 2.2_kg << std::endl;
    return 0;
}
