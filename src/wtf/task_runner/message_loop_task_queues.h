//
// Created by admin on 2021-07-02.
//

#ifndef TEST_MESSAGE_LOOP_TASK_QUEUES_H
#define TEST_MESSAGE_LOOP_TASK_QUEUES_H

#include <map>
#include <mutex>

#include "task_queue_id.h"
#include "task.h"
#include "task_source.h"
#include "task_source_grade.h"
#include "wakeable.h"
#include "time/time_point.h"
#include "wtf/macros.h"

namespace wtf {

class TaskQueueEntry {
public:
    using TaskObservers = std::map<intptr_t, wtf::Task>;
    Wakeable* wakeable;
    TaskObservers task_observers;
    std::unique_ptr<TaskSource> task_source;

    TaskQueueId owner_of;
    TaskQueueId subsumed_by;
    TaskQueueId created_for;

    explicit TaskQueueEntry(TaskQueueId created_for);

private:
    WTF_DISALLOW_COPY_ASSIGN_AND_MOVE(TaskQueueEntry);
};

class MessageLoopTaskQueues {
public:

    static MessageLoopTaskQueues* GetInstance();

    TaskQueueId CreateTaskQueue();

    void Dispose(TaskQueueId queue_id);

    void DisposeTasks(TaskQueueId queue_id);

    void RegisterTask(TaskQueueId queue_id,
                      const wtf::Task& task,
                      wtf::TimePoint target_time,
                      wtf::TaskSourceGrade task_source_grade = wtf::TaskSourceGrade::Unspecified);

    bool HasPendingTasks(TaskQueueId queue_id) const;

    wtf::Task GetNextTaskToRun(TaskQueueId queue_id, wtf::TimePoint from_time);

    size_t GetNumPendingTasks(TaskQueueId queue_id) const;

    static TaskSourceGrade GetCurrentTaskSourceGrade();

    void AddTaskObserver(TaskQueueId queue_id,
                         intptr_t key,
                         const wtf::Task& callback);

    void RemoveTaskObserver(TaskQueueId queue_id, intptr_t key);

    std::vector<wtf::Task> GetObserversToNotify(TaskQueueId queue_id) const;

    void SetWakeable(TaskQueueId queue_id, wtf::Wakeable* wakeable);

    bool Merge(TaskQueueId owner, TaskQueueId subsumed);

    bool Unmerge(TaskQueueId owner);

    bool Owns(TaskQueueId owner, TaskQueueId subsumed) const;

    TaskQueueId GetSubsumedTaskQueueId(TaskQueueId owner) const;

    void PauseSecondarySource(TaskQueueId queue_id);

    void ResumeSecondarySource(TaskQueueId queue_id);

private:
    class MergedQueuesRunner;

    MessageLoopTaskQueues() = default;

    ~MessageLoopTaskQueues();

    void WakeUpUnlocked(TaskQueueId queue_id, wtf::TimePoint time) const;

    bool HasPendingTasksUnlocked(TaskQueueId queue_id) const;

    TaskSource::TopTask PeekNextTaskUnlocked(TaskQueueId owner) const;

    wtf::TimePoint GetNextWakeTimeUnlocked(TaskQueueId queue_id) const;

    static std::mutex creation_mutex_;

    mutable std::mutex queue_mutex_;
    std::map<TaskQueueId, std::unique_ptr<TaskQueueEntry>> queue_entries_;

    size_t task_queue_id_counter_ = 0;

    std::atomic_int order_ = 0;

    WTF_DISALLOW_COPY_ASSIGN_AND_MOVE(MessageLoopTaskQueues);

};
enum class FlushType {
    Single,
    All,
};
} // namespace wtf

#endif //TEST_MESSAGE_LOOP_TASK_QUEUES_H
