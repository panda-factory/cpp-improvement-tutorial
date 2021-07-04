//
// Created by admin on 2021-07-02.
//

#include "message_loop_task_queues.h"

#include <memory>

#include "logging/logging.h"
#include "thread_local.h"

namespace wtf {

class TaskSourceGradeHolder {
public:
    TaskSourceGrade task_source_grade;

    explicit TaskSourceGradeHolder(TaskSourceGrade task_source_grade_arg)
            : task_source_grade(task_source_grade_arg)
    {}
};

WTF_THREAD_LOCAL std::unique_ptr<TaskSourceGradeHolder> tls_task_source_grade;


std::mutex MessageLoopTaskQueues::creation_mutex_;
const size_t TaskQueueId::unmerged = ULONG_MAX;
static const TaskQueueId g_unmerged = TaskQueueId(TaskQueueId::unmerged);

TaskQueueEntry::TaskQueueEntry(TaskQueueId created_for_arg)
        : owner_of(g_unmerged),
          subsumed_by(g_unmerged),
          created_for(created_for_arg)
{
    wakeable = NULL;
    task_observers = TaskObservers();
    task_source = std::make_unique<TaskSource>(created_for);
}

MessageLoopTaskQueues *MessageLoopTaskQueues::GetInstance()
{
    static MessageLoopTaskQueues *instance = nullptr;
    if (!instance) {
        instance = new MessageLoopTaskQueues();
    }
    return instance;
}

void MessageLoopTaskQueues::AddTaskObserver(TaskQueueId queue_id,
                                            intptr_t key,
                                            const wtf::Task &callback)
{
    std::lock_guard guard(queue_mutex_);
    WTF_DCHECK(callback != nullptr) << "Observer callback must be non-null.";
    queue_entries_.at(queue_id)->task_observers[key] = callback;
}

TaskQueueId MessageLoopTaskQueues::CreateTaskQueue()
{
    std::scoped_lock locker(queue_mutex_);
    TaskQueueId loop_id = TaskQueueId(task_queue_id_counter_);
    ++task_queue_id_counter_;
    queue_entries_[loop_id] = std::make_unique<TaskQueueEntry>(loop_id);
    return loop_id;
}

void MessageLoopTaskQueues::Dispose(TaskQueueId queue_id)
{
    std::scoped_lock locker(queue_mutex_);
    const auto &queue_entry = queue_entries_.at(queue_id);
    WTF_DCHECK(queue_entry->subsumed_by == g_unmerged);
    TaskQueueId subsumed = queue_entry->owner_of;
    queue_entries_.erase(queue_id);
    if (subsumed != g_unmerged) {
        queue_entries_.erase(subsumed);
    }
}

void MessageLoopTaskQueues::DisposeTasks(TaskQueueId queue_id)
{
    std::lock_guard guard(queue_mutex_);
    const auto &queue_entry = queue_entries_.at(queue_id);
    WTF_DCHECK(queue_entry->subsumed_by == g_unmerged);
    TaskQueueId subsumed = queue_entry->owner_of;
    queue_entry->task_source->ShutDown();
    if (subsumed != g_unmerged) {
        queue_entries_.at(subsumed)->task_source->ShutDown();
    }
}

std::vector<wtf::Task> MessageLoopTaskQueues::GetObserversToNotify(
        TaskQueueId queue_id) const
{
    std::lock_guard guard(queue_mutex_);
    std::vector<wtf::Task> observers;

    if (queue_entries_.at(queue_id)->subsumed_by != g_unmerged) {
        return observers;
    }

    for (const auto &observer : queue_entries_.at(queue_id)->task_observers) {
        observers.push_back(observer.second);
    }

    TaskQueueId subsumed = queue_entries_.at(queue_id)->owner_of;
    if (subsumed != g_unmerged) {
        for (const auto &observer : queue_entries_.at(subsumed)->task_observers) {
            observers.push_back(observer.second);
        }
    }

    return observers;
}

wtf::Task MessageLoopTaskQueues::GetNextTaskToRun(TaskQueueId queue_id,
                                                  wtf::TimePoint from_time)
{
    std::lock_guard guard(queue_mutex_);
    if (!HasPendingTasksUnlocked(queue_id)) {
        return nullptr;
    }
    TaskSource::TopTask top = PeekNextTaskUnlocked(queue_id);

    if (!HasPendingTasksUnlocked(queue_id)) {
        WakeUpUnlocked(queue_id, wtf::TimePoint::Max());
    } else {
        WakeUpUnlocked(queue_id, GetNextWakeTimeUnlocked(queue_id));
    }

    if (top.task.GetTargetTime() > from_time) {
        return nullptr;
    }
    wtf::Task invocation = top.task.GetTask();
    queue_entries_.at(top.task_queue_id)
            ->task_source->PopTask(top.task.GetTaskSourceGrade());
    {
        std::scoped_lock creation(creation_mutex_);
        const auto task_source_grade = top.task.GetTaskSourceGrade();
        tls_task_source_grade.reset(new TaskSourceGradeHolder{task_source_grade});
    }
    return invocation;
}

wtf::TimePoint MessageLoopTaskQueues::GetNextWakeTimeUnlocked(
        TaskQueueId queue_id) const
{
    return PeekNextTaskUnlocked(queue_id).task.GetTargetTime();
}

bool MessageLoopTaskQueues::HasPendingTasksUnlocked(
        TaskQueueId queue_id) const
{
    const auto &entry = queue_entries_.at(queue_id);
    bool is_subsumed = entry->subsumed_by != g_unmerged;
    if (is_subsumed) {
        return false;
    }

    if (!entry->task_source->IsEmpty()) {
        return true;
    }

    const TaskQueueId subsumed = entry->owner_of;
    if (subsumed == g_unmerged) {
        // this is not an owner and queue is empty.
        return false;
    } else {
        return !queue_entries_.at(subsumed)->task_source->IsEmpty();
    }
}

bool MessageLoopTaskQueues::Owns(TaskQueueId owner,
                                 TaskQueueId subsumed) const
{
    std::lock_guard guard(queue_mutex_);
    return owner != g_unmerged && subsumed != g_unmerged &&
           subsumed == queue_entries_.at(owner)->owner_of;
}

TaskSource::TopTask MessageLoopTaskQueues::PeekNextTaskUnlocked(
        TaskQueueId owner) const
{
    WTF_DCHECK(HasPendingTasksUnlocked(owner));
    const auto &entry = queue_entries_.at(owner);
    const TaskQueueId subsumed = entry->owner_of;
    if (subsumed == g_unmerged) {
        return entry->task_source->Top();
    }

    TaskSource *owner_tasks = entry->task_source.get();
    TaskSource *subsumed_tasks = queue_entries_.at(subsumed)->task_source.get();

    // we are owning another task queue
    const bool subsumed_has_task = !subsumed_tasks->IsEmpty();
    const bool owner_has_task = !owner_tasks->IsEmpty();
    wtf::TaskQueueId top_queue_id = owner;
    if (owner_has_task && subsumed_has_task) {
        const auto owner_task = owner_tasks->Top();
        const auto subsumed_task = subsumed_tasks->Top();
        if (owner_task.task > subsumed_task.task) {
            top_queue_id = subsumed;
        } else {
            top_queue_id = owner;
        }
    } else if (owner_has_task) {
        top_queue_id = owner;
    } else {
        top_queue_id = subsumed;
    }
    return queue_entries_.at(top_queue_id)->task_source->Top();
}

void MessageLoopTaskQueues::RegisterTask(
        TaskQueueId queue_id,
        const wtf::Task &task,
        wtf::TimePoint target_time,
        wtf::TaskSourceGrade task_source_grade)
{
    std::lock_guard guard(queue_mutex_);
    size_t order = order_++;
    const auto &queue_entry = queue_entries_.at(queue_id);
    queue_entry->task_source->RegisterTask(
            {order, task, target_time, task_source_grade});
    TaskQueueId loop_to_wake = queue_id;

    if (queue_entry->subsumed_by != g_unmerged) {
        loop_to_wake = queue_entry->subsumed_by;
    }

    // This can happen when the secondary tasks are paused.
    if (HasPendingTasksUnlocked(loop_to_wake)) {
        WakeUpUnlocked(loop_to_wake, GetNextWakeTimeUnlocked(loop_to_wake));
    }
}

void MessageLoopTaskQueues::SetWakeable(TaskQueueId queue_id,
                                        wtf::Wakeable *wakeable)
{
    std::lock_guard guard(queue_mutex_);
    WTF_CHECK(!queue_entries_.at(queue_id)->wakeable)
    << "Wakeable can only be set once.";
    queue_entries_.at(queue_id)->wakeable = wakeable;
}

void MessageLoopTaskQueues::WakeUpUnlocked(TaskQueueId queue_id,
                                           wtf::TimePoint time) const
{
    if (queue_entries_.at(queue_id)->wakeable) {
        queue_entries_.at(queue_id)->wakeable->WakeUp(time);
    }
}
} // namespace wtf