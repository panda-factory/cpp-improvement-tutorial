// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "message_loop.h"

#include <iostream>
#include <thread>

#include "sync/waitable_event.h"
#include "gtest/gtest.h"

#define TIMESENSITIVE(x) TimeSensitiveTest_##x
#if OS_WIN
#define PLATFORM_SPECIFIC_CAPTURE(...) [ __VA_ARGS__, count ]
#else
#define PLATFORM_SPECIFIC_CAPTURE(...) [__VA_ARGS__]
#endif

TEST(MessageLoop, GetCurrent)
{
    std::thread thread([]() {
        wtf::MessageLoop::EnsureInitializedForCurrentThread();
        ASSERT_TRUE(wtf::MessageLoop::GetCurrent().GetTaskRunner());
    });
    thread.join();
}

TEST(MessageLoop, DifferentThreadsHaveDifferentLoops)
{
    wtf::MessageLoop *loop1 = nullptr;
    wtf::OneOffWaitableEvent latch1;
    wtf::OneOffWaitableEvent term1;
    std::thread thread1([&loop1, &latch1, &term1]() {
        wtf::MessageLoop::EnsureInitializedForCurrentThread();
        loop1 = &wtf::MessageLoop::GetCurrent();
        latch1.Signal();
        term1.Wait();
    });

    wtf::MessageLoop *loop2 = nullptr;
    wtf::OneOffWaitableEvent latch2;
    wtf::OneOffWaitableEvent term2;
    std::thread thread2([&loop2, &latch2, &term2]() {
        wtf::MessageLoop::EnsureInitializedForCurrentThread();
        loop2 = &wtf::MessageLoop::GetCurrent();
        latch2.Signal();
        term2.Wait();
    });
    latch1.Wait();
    latch2.Wait();
    ASSERT_FALSE(loop1 == loop2);
    term1.Signal();
    term2.Signal();
    thread1.join();
    thread2.join();
}

TEST(MessageLoop, CanRunAndTerminate)
{
    bool started = false;
    bool terminated = false;
    std::thread thread([&started, &terminated]() {
        wtf::MessageLoop::EnsureInitializedForCurrentThread();
        auto &loop = wtf::MessageLoop::GetCurrent();
        ASSERT_TRUE(loop.GetTaskRunner());
        loop.GetTaskRunner()->PostTask([&terminated]() {
            wtf::MessageLoop::GetCurrent().Terminate();
            terminated = true;
        });
        loop.Run();
        started = true;
    });
    thread.join();
    ASSERT_TRUE(started);
    ASSERT_TRUE(terminated);
}

TEST(MessageLoop, NonDelayedTasksAreRunInOrder)
{
    const size_t count = 100;
    bool started = false;
    bool terminated = false;
    std::thread thread([&started, &terminated, count]() {
        wtf::MessageLoop::EnsureInitializedForCurrentThread();
        auto &loop = wtf::MessageLoop::GetCurrent();
        size_t current = 0;
        for (size_t i = 0; i < count; i++) {
            loop.GetTaskRunner()->PostTask(
                    PLATFORM_SPECIFIC_CAPTURE(&terminated, i, &current)() {
                        ASSERT_EQ(current, i);
                        current++;
                        if (count == i + 1) {
                            wtf::MessageLoop::GetCurrent().Terminate();
                            terminated = true;
                        }
                    });
        }
        loop.Run();
        ASSERT_EQ(current, count);
        started = true;
    });
    thread.join();
    ASSERT_TRUE(started);
    ASSERT_TRUE(terminated);
}

TEST(MessageLoop, DelayedTasksAtSameTimeAreRunInOrder)
{
    const size_t count = 100;
    bool started = false;
    bool terminated = false;
    std::thread thread([&started, &terminated, count]() {
        wtf::MessageLoop::EnsureInitializedForCurrentThread();
        auto &loop = wtf::MessageLoop::GetCurrent();
        size_t current = 0;
        const auto now_plus_some =
                wtf::TimePoint::Now() + wtf::TimeDelta::FromMilliseconds(2);
        for (size_t i = 0; i < count; i++) {
            loop.GetTaskRunner()->PostTaskForTime(
                    PLATFORM_SPECIFIC_CAPTURE(&terminated, i, &current)() {
                        ASSERT_EQ(current, i);
                        current++;
                        if (count == i + 1) {
                            wtf::MessageLoop::GetCurrent().Terminate();
                            terminated = true;
                        }
                    },
                    now_plus_some);
        }
        loop.Run();
        ASSERT_EQ(current, count);
        started = true;
    });
    thread.join();
    ASSERT_TRUE(started);
    ASSERT_TRUE(terminated);
}

TEST(MessageLoop, CheckRunsTaskOnCurrentThread)
{
    wtf::TaskRunner *runner;
    wtf::OneOffWaitableEvent latch;
    std::thread thread([&runner, &latch]() {
        wtf::MessageLoop::EnsureInitializedForCurrentThread();
        auto &loop = wtf::MessageLoop::GetCurrent();
        runner = loop.GetTaskRunner();
        latch.Signal();
        loop.Run();
        ASSERT_TRUE(loop.GetTaskRunner()->RunsTasksOnCurrentThread());
    });
    latch.Wait();
    ASSERT_TRUE(runner);
    ASSERT_FALSE(runner->RunsTasksOnCurrentThread());
    runner->PostTask([]() {
        wtf::MessageLoop::GetCurrent().Terminate();
    });
    thread.join();
}

TEST(MessageLoop, TIMESENSITIVE(SingleDelayedTaskByDelta)) {
  bool checked = false;
  std::thread thread([&checked]() {
    wtf::MessageLoop::EnsureInitializedForCurrentThread();
    auto& loop = wtf::MessageLoop::GetCurrent();
    auto begin = wtf::TimePoint::Now();
    loop.GetTaskRunner()->PostDelayedTask(
        [begin, &checked]() {
          auto delta = wtf::TimePoint::Now() - begin;
          auto ms = delta.ToMillisecondsF();
          ASSERT_GE(ms, 3);
          // TODO ASSERT_LE(ms, 7);
          checked = true;
          wtf::MessageLoop::GetCurrent().Terminate();
        },
        wtf::TimeDelta::FromMilliseconds(5));
    loop.Run();
  });
  thread.join();
  ASSERT_TRUE(checked);
}

TEST(MessageLoop, TIMESENSITIVE(SingleDelayedTaskForTime)) {
  bool checked = false;
  std::thread thread([&checked]() {
    wtf::MessageLoop::EnsureInitializedForCurrentThread();
    auto& loop = wtf::MessageLoop::GetCurrent();
    auto begin = wtf::TimePoint::Now();
    loop.GetTaskRunner()->PostTaskForTime(
        [begin, &checked]() {
          auto delta = wtf::TimePoint::Now() - begin;
          auto ms = delta.ToMillisecondsF();
          ASSERT_GE(ms, 3);
            // TODO ASSERT_LE(ms, 7);
          checked = true;
          wtf::MessageLoop::GetCurrent().Terminate();
        },
        wtf::TimePoint::Now() + wtf::TimeDelta::FromMilliseconds(5));
    loop.Run();
  });
  thread.join();
  ASSERT_TRUE(checked);
}

TEST(MessageLoop, TIMESENSITIVE(MultipleDelayedTasksWithIncreasingDeltas)) {
  const auto count = 10;
  int checked = false;
  std::thread thread(PLATFORM_SPECIFIC_CAPTURE(&checked)() {
    wtf::MessageLoop::EnsureInitializedForCurrentThread();
    auto& loop = wtf::MessageLoop::GetCurrent();
    for (int target_ms = 0 + 2; target_ms < count + 2; target_ms++) {
      auto begin = wtf::TimePoint::Now();
      loop.GetTaskRunner()->PostDelayedTask(
          PLATFORM_SPECIFIC_CAPTURE(begin, target_ms, &checked)() {
            auto delta = wtf::TimePoint::Now() - begin;
            auto ms = delta.ToMillisecondsF();
            ASSERT_GE(ms, target_ms - 2);
              // TODO ASSERT_LE(ms, target_ms + 2);
            checked++;
            if (checked == count) {
              wtf::MessageLoop::GetCurrent().Terminate();
            }
          },
          wtf::TimeDelta::FromMilliseconds(target_ms));
    }
    loop.Run();
  });
  thread.join();
  ASSERT_EQ(checked, count);
}

TEST(MessageLoop, TIMESENSITIVE(MultipleDelayedTasksWithDecreasingDeltas)) {
  const auto count = 10;
  int checked = false;
  std::thread thread(PLATFORM_SPECIFIC_CAPTURE(&checked)() {
    wtf::MessageLoop::EnsureInitializedForCurrentThread();
    auto& loop = wtf::MessageLoop::GetCurrent();
    for (int target_ms = count + 2; target_ms > 0 + 2; target_ms--) {
      auto begin = wtf::TimePoint::Now();
      loop.GetTaskRunner()->PostDelayedTask(
          PLATFORM_SPECIFIC_CAPTURE(begin, target_ms, &checked)() {
            auto delta = wtf::TimePoint::Now() - begin;
            auto ms = delta.ToMillisecondsF();
            ASSERT_GE(ms, target_ms - 2);
              // TODO ASSERT_LE(ms, target_ms + 2);
            checked++;
            if (checked == count) {
              wtf::MessageLoop::GetCurrent().Terminate();
            }
          },
          wtf::TimeDelta::FromMilliseconds(target_ms));
    }
    loop.Run();
  });
  thread.join();
  ASSERT_EQ(checked, count);
}

TEST(MessageLoop, TaskObserverFire) {
  bool started = false;
  bool terminated = false;
  std::thread thread([&started, &terminated]() {
    wtf::MessageLoop::EnsureInitializedForCurrentThread();
    const size_t count = 25;
    auto& loop = wtf::MessageLoop::GetCurrent();
    size_t task_count = 0;
    size_t obs_count = 0;
    auto obs = PLATFORM_SPECIFIC_CAPTURE(&obs_count)() { obs_count++; };
    for (size_t i = 0; i < count; i++) {
      loop.GetTaskRunner()->PostTask(
          PLATFORM_SPECIFIC_CAPTURE(&terminated, i, &task_count)() {
            ASSERT_EQ(task_count, i);
            task_count++;
            if (count == i + 1) {
              wtf::MessageLoop::GetCurrent().Terminate();
              terminated = true;
            }
          });
    }
    loop.AddTaskObserver(0, obs);
    loop.Run();
    ASSERT_EQ(task_count, count);
    ASSERT_EQ(obs_count, count);
    started = true;
  });
  thread.join();
  ASSERT_TRUE(started);
  ASSERT_TRUE(terminated);
}