//
// Created by admin on 2021-05-17.
//

#ifndef TEST_DLL_TASK_H
#define TEST_DLL_TASK_H

#include <functional>
#include <string>

class Task {
public:
    using TaskCompleteCallback = std::function<void ()>;
    using TaskErrorCallback = std::function<void()>;
    void dispatch(std::string event, void* data);
private:
    static size_t taskId = 0;

    void* source_;
    void* output_;
    void* input_;
    bool isFinish_;
    size_t id_;
};


#endif //TEST_DLL_TASK_H
