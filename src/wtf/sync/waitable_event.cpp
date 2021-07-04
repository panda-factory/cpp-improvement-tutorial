//
// Created by admin on 2021-07-02.
//

#include "waitable_event.h"
#include "logging/logging.h"
#include "wtf/scope_exit.h"

namespace wtf {

void OneOffWaitableEvent::Signal() {
    std::scoped_lock locker(mutex_);
    signaled_ = true;
    cv_.notify_one();
}

void OneOffWaitableEvent::Reset() {
    std::scoped_lock locker(mutex_);
    signaled_ = false;
}

void OneOffWaitableEvent::Wait() {
    std::unique_lock<std::mutex> locker(mutex_);
    SCOPE_GUARD {
        signaled_ = false;
    };

    while (!signaled_) {
        cv_.wait(locker);
    }
}

bool OneOffWaitableEvent::WaitWithTimeout(TimeDelta timeout) {
    std::unique_lock<std::mutex> locker(mutex_);
    SCOPE_GUARD {
        signaled_ = false;
    };

    if (signaled_) {
        return false;
    }

    // We may get spurious wakeups.
    TimeDelta wait_remaining = timeout;
    TimePoint start = TimePoint::Now();
    while (true) {
        if (std::cv_status::timeout ==
            cv_.wait_for(
                    locker, std::chrono::nanoseconds(wait_remaining.ToNanoseconds()))) {
            return true;  // Definitely timed out.
        }

        // We may have been awoken.
        if (signaled_) {
            break;
        }

        // Or the wakeup may have been spurious.
        TimePoint now = TimePoint::Now();
        WTF_DCHECK(now >= start);
        TimeDelta elapsed = now - start;
        // It's possible that we may have timed out anyway.
        if (elapsed >= timeout) {
            return true;
        }

        // Otherwise, recalculate the amount that we have left to wait.
        wait_remaining = timeout - elapsed;
    }

    return false;
}

bool OneOffWaitableEvent::IsSignaled() {
    std::scoped_lock locker(mutex_);
    return signaled_;
}

} // namespace wtf