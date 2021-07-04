//
// Created by admin on 2021-07-04.
//

#include "gtest/gtest.h"
#include "task_runner/thread.h"
#include "logging/logging.h"
#include "task_runner/message_loop.h"

TEST(Thread, SimpleInitialization) {
    wtf::Thread log_thread("log.task");

    log_thread.GetTaskRunner().PostTask([] () {
        wtf::MessageLoop::GetCurrent().Terminate();
    });
}