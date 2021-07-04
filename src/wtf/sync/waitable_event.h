//
// Created by admin on 2021-07-02.
//

#ifndef TEST_WAITABLE_EVENT_H
#define TEST_WAITABLE_EVENT_H

#include <condition_variable>
#include <mutex>

#include "time/time_point.h"
#include "wtf/macros.h"

namespace wtf {

class WaitableEvent {
public:
    virtual void Signal() = 0;

    virtual void Reset() = 0;

    virtual void Wait() = 0;

    // return true when signal is timeout.
    virtual bool WaitWithTimeout(wtf::TimeDelta timeout) = 0;

    virtual bool IsSignaled() = 0;

protected:
    WaitableEvent() = default;

    virtual ~WaitableEvent() = default;

    std::condition_variable cv_;
    std::mutex mutex_;
    bool signaled_ = false;

private:
};

class OneOffWaitableEvent final : public WaitableEvent {
public:
    void Signal() override;

    void Reset() override;

    void Wait() override;

    bool WaitWithTimeout(wtf::TimeDelta timeout) override;

    bool IsSignaled() override;

    OneOffWaitableEvent() = default;

private:

    WTF_DISALLOW_COPY_AND_ASSIGN(OneOffWaitableEvent);
};

} // namespace wtf


#endif //TEST_WAITABLE_EVENT_H
