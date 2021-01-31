//
// Created by admin on 2021/1/17.
//

#include <iostream>

#include "gtest/gtest.h"
#include "core/log_setting.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    core::SetLogSettings(core::LogSettings{core::LOG_INFO});

    int ret = RUN_ALL_TESTS();
    return 0;
}