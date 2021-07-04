//
// Created by admin on 2021-07-03.
//

#ifndef TEST_TASK_SOURCE_H
#define TEST_TASK_SOURCE_H

#include "delayed_task.h"
#include "task_queue_id.h"

#include "wtf/macros.h"

namespace wtf {

class TaskSource {
public:
    struct TopTask {
        TaskQueueId task_queue_id;
        const DelayedTask& task;
    };
    explicit TaskSource(TaskQueueId task_queue_id);

    ~TaskSource();

    /// Drops the pending tasks from both primary and secondary task heaps.
    void ShutDown();

    /// Adds a task to the corresponding task heap as dictated by the
    /// `TaskSourceGrade` of the `DelayedTask`.
    void RegisterTask(const DelayedTask& task);

    /// Pops the task heap corresponding to the `TaskSourceGrade`.
    void PopTask(TaskSourceGrade grade);

    /// Returns the number of pending tasks. Excludes the tasks from the secondary
    /// heap if it's paused.
    size_t GetNumPendingTasks() const;

    /// Returns true if `GetNumPendingTasks` is zero.
    bool IsEmpty() const;

    /// Returns the top task based on scheduled time, taking into account whether
    /// the secondary heap has been paused or not.
    TopTask Top() const;

    /// Pause providing tasks from secondary task heap.
    void PauseSecondary();

    /// Resume providing tasks from secondary task heap.
    void ResumeSecondary();

private:
    const wtf::TaskQueueId task_queue_id_;
    wtf::DelayedTaskQueue primary_task_queue_;
    wtf::DelayedTaskQueue secondary_task_queue_;
    int secondary_pause_requests_ = 0;

    WTF_DISALLOW_COPY_ASSIGN_AND_MOVE(TaskSource);
};

} // namespace wtf



#endif //TEST_TASK_SOURCE_H
