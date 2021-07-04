//
// Created by admin on 2021-07-04.
//

#include "gtest/gtest.h"
#include "task_runner/thread.h"
#include "logging/logging.h"

TEST(Thread, SimpleInitialization) {
    wtf::Thread log_thread("log.task");
    WTF_LOG(INFO) << "gzx";

    log_thread.GetTaskRunner().PostTask([] () {
        WTF_LOG(INFO) << "gzx";
    });
}