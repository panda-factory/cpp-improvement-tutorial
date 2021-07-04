//
// Created by admin on 2021-07-03.
//

#ifndef TEST_TASK_SOURCE_GRADE_H
#define TEST_TASK_SOURCE_GRADE_H

namespace wtf {
enum class TaskSourceGrade {
    // This `TaskSourceGrade` indicates that a task is critical to user
    // interaction.
    UserCustom,
    // The absence of a specialized `TaskSourceGrade`.
    Unspecified,
};
} // namespace wtf

#endif //TEST_TASK_SOURCE_GRADE_H
