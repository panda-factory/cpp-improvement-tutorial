//
// Created by admin on 2021-07-03.
//

#include "task_source.h"

#include "logging/logging.h"

namespace wtf {

TaskSource::TaskSource(TaskQueueId task_queue_id)
        : task_queue_id_(task_queue_id) {}

TaskSource::~TaskSource() {
    ShutDown();
}

size_t TaskSource::GetNumPendingTasks() const {
    size_t size = primary_task_queue_.size();
    if (secondary_pause_requests_ == 0) {
        size += secondary_task_queue_.size();
    }
    return size;
}

bool TaskSource::IsEmpty() const {
    return GetNumPendingTasks() == 0;
}

void TaskSource::RegisterTask(const DelayedTask& task) {
    switch (task.GetTaskSourceGrade()) {
        case TaskSourceGrade::UserCustom:
            primary_task_queue_.push(task);
            break;
        case TaskSourceGrade::Unspecified:
            primary_task_queue_.push(task);
            break;
    }
}

void TaskSource::PopTask(TaskSourceGrade grade) {
    switch (grade) {
        case TaskSourceGrade::UserCustom:
            primary_task_queue_.pop();
            break;
        case TaskSourceGrade::Unspecified:
            primary_task_queue_.pop();
            break;
    }
}

void TaskSource::ShutDown() {
    primary_task_queue_ = {};
    secondary_task_queue_ = {};
}

TaskSource::TopTask TaskSource::Top() const {
    WTF_CHECK(!IsEmpty());
    if (secondary_pause_requests_ > 0 || secondary_task_queue_.empty()) {
        const auto& primary_top = primary_task_queue_.top();
        return {task_queue_id_, primary_top};
    } else if (primary_task_queue_.empty()) {
        const auto& secondary_top = secondary_task_queue_.top();
        return {task_queue_id_, secondary_top,
        };
    } else {
        const auto& primary_top = primary_task_queue_.top();
        const auto& secondary_top = secondary_task_queue_.top();
        if (primary_top > secondary_top) {
            return {task_queue_id_, secondary_top,
            };
        } else {
            return {task_queue_id_, primary_top,
            };
        }
    }
}
} // namespace wtf