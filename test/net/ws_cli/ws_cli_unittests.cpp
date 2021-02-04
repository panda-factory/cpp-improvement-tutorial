//
// Created by admin on 2021/2/4.
//

#include "gtest/gtest.h"
#include "core/log_setting.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    core::SetLogSettings(core::LogSettings{core::LOG_INFO});

    int ret = RUN_ALL_TESTS();
    return 0;
}