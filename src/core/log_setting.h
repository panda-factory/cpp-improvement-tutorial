//
// Created by admin on 2021/1/28.
//

#ifndef TEST_LOG_SETTING_H
#define TEST_LOG_SETTING_H
#include "log_level.h"
namespace core {

// Settings which control the behavior of FML logging.
struct LogSettings {

    LogSeverity min_log_level = LOG_INFO;
};

// Gets the active log settings for the current process.
void SetLogSettings(const LogSettings& settings);

// Sets the active log settings for the current process.
LogSettings GetLogSettings();

// Gets the minimum log level for the current process. Never returs a value
// higher than LOG_FATAL.
int GetMinLogLevel();

class ScopedSetLogSettings {
public:
    ScopedSetLogSettings(const LogSettings& settings);
    ~ScopedSetLogSettings();

private:
    LogSettings old_settings_;
};

} // namespace core


#endif //TEST_LOG_SETTING_H
