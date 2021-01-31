//
// Created by admin on 2021/1/17.
//

#include <iostream>

#include "gtest/gtest.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    std::cout << "unit tests" << std::endl;

    int ret = RUN_ALL_TESTS();
    return 0;
}
