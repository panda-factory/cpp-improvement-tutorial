//
// Created by admin on 2021-05-30.
//

#include "pipeline.h"

size_t GetNextPipelineTraceID() {
    static std::atomic_size_t PipelineLastTraceID = {0};
    return ++PipelineLastTraceID;
}
