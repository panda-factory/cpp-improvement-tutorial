//
// Created by admin on 2021-05-30.
//

#ifndef TEST_PIPELINE_H
#define TEST_PIPELINE_H

#include <deque>
#include <memory>
#include <mutex>
#include <functional>

#include "wtf/macros.h"
#include "wtf/synchronization/semaphore.h"

enum class PipelineConsumeResult {
    NoneAvailable,
    Done,
    MoreAvailable,
};

size_t GetNextPipelineTraceID();

/// A thread-safe queue of resources for a single consumer and a single
/// producer.
template <class R>
class Pipeline {
public:
    using Resource = R;
    using ResourcePtr = std::unique_ptr<Resource>;

    /// Denotes a spot in the pipeline reserved for the producer to finish
    /// preparing a completed pipeline resource.
    class ProducerContinuation {
    public:
        ProducerContinuation() : trace_id_(0) {}

        ProducerContinuation(ProducerContinuation&& other)
                : continuation_(other.continuation_), trace_id_(other.trace_id_) {
            other.continuation_ = nullptr;
            other.trace_id_ = 0;
        }

        ProducerContinuation& operator=(ProducerContinuation&& other) {
            std::swap(continuation_, other.continuation_);
            std::swap(trace_id_, other.trace_id_);
            return *this;
        }

        ~ProducerContinuation() {
            if (continuation_) {
                continuation_(nullptr, trace_id_);
            }
        }

        [[nodiscard]] bool Complete(ResourcePtr resource) {
            bool result = false;
            if (continuation_) {
                result = continuation_(std::move(resource), trace_id_);
                continuation_ = nullptr;
            }
            return result;
        }

        operator bool() const { return continuation_ != nullptr; }

    private:
        friend class Pipeline;
        using Continuation = std::function<bool(ResourcePtr, size_t)>;

        Continuation continuation_;
        size_t trace_id_;

        ProducerContinuation(const Continuation& continuation, size_t trace_id)
                : continuation_(continuation), trace_id_(trace_id) {
        }

        WTF_DISALLOW_COPY_AND_ASSIGN(ProducerContinuation);
    };

    explicit Pipeline(uint32_t depth)
            : depth_(depth), empty_(depth), available_(0), inflight_(0) {}

    ~Pipeline() = default;

    bool IsValid() const { return empty_.IsValid() && available_.IsValid(); }

    ProducerContinuation Produce() {
        if (!empty_.TryWait()) {
            return {};
        }
        ++inflight_;

        return ProducerContinuation{
                std::bind(&Pipeline::ProducerCommit, this, std::placeholders::_1,
                          std::placeholders::_2),  // continuation
                GetNextPipelineTraceID()};         // trace id
    }

    // Create a `ProducerContinuation` that will only push the task if the queue
    // is empty.
    // Prefer using |Produce|. ProducerContinuation returned by this method
    // doesn't guarantee that the frame will be rendered.
    ProducerContinuation ProduceIfEmpty() {
        if (!empty_.TryWait()) {
            return {};
        }
        ++inflight_;

        return ProducerContinuation{
                std::bind(&Pipeline::ProducerCommitIfEmpty, this, std::placeholders::_1,
                          std::placeholders::_2),  // continuation
                GetNextPipelineTraceID()};         // trace id
    }

    using Consumer = std::function<void(ResourcePtr)>;

    /// @note Procedure doesn't copy all TaskTypes.
    [[nodiscard]] PipelineConsumeResult Consume(const Consumer& consumer) {
        if (consumer == nullptr) {
            return PipelineConsumeResult::NoneAvailable;
        }

        if (!available_.TryWait()) {
            return PipelineConsumeResult::NoneAvailable;
        }

        ResourcePtr resource;
        size_t trace_id = 0;
        size_t items_count = 0;

        {
            std::scoped_lock lock(queue_mutex_);
            std::tie(resource, trace_id) = std::move(queue_.front());
            queue_.pop_front();
            items_count = queue_.size();
        }

        {
            consumer(std::move(resource));
        }

        empty_.Signal();
        --inflight_;

        return items_count > 0 ? PipelineConsumeResult::MoreAvailable
                               : PipelineConsumeResult::Done;
    }

private:
    const uint32_t depth_;
    wtf::Semaphore empty_;
    wtf::Semaphore available_;
    std::atomic<int> inflight_;
    std::mutex queue_mutex_;
    std::deque<std::pair<ResourcePtr, size_t>> queue_;

    bool ProducerCommit(ResourcePtr resource, size_t trace_id) {
        {
            std::scoped_lock lock(queue_mutex_);
            queue_.emplace_back(std::move(resource), trace_id);
        }

        // Ensure the queue mutex is not held as that would be a pessimization.
        available_.Signal();
        return true;
    }

    bool ProducerCommitIfEmpty(ResourcePtr resource, size_t trace_id) {
        {
            std::scoped_lock lock(queue_mutex_);
            if (!queue_.empty()) {
                // Bail if the queue is not empty, opens up spaces to produce other
                // frames.
                empty_.Signal();
                return false;
            }
            queue_.emplace_back(std::move(resource), trace_id);
        }

        // Ensure the queue mutex is not held as that would be a pessimization.
        available_.Signal();
        return true;
    }

    WTF_DISALLOW_COPY_AND_ASSIGN(Pipeline);
};



#endif //TEST_PIPELINE_H
