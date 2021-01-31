//
// Created by admin on 2021/1/28.
//

#ifndef TEST_LOGGING_H
#define TEST_LOGGING_H
#include <sstream>

#include "log_level.h"
#include "macros.h"

namespace core {

class LogMessageVoidify {
public:
    void operator&(std::ostream&) {}
};

class LogMessage {
public:
    LogMessage(LogSeverity severity,
               const char* file,
               int line,
               const char* condition);
    ~LogMessage();

    std::ostream& stream() { return stream_; }

private:
    std::ostringstream stream_;
    const LogSeverity severity_;
    const char* file_;
    const int line_;

    CORE_DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

int GetVlogVerbosity();

bool ShouldCreateLogMessage(LogSeverity severity);

[[noreturn]] void KillProcess();

} // namespace core

#define CORE_LOG_STREAM(severity) \
  ::core::LogMessage(::core::LOG_##severity, __FILE__, __LINE__, nullptr).stream()

#define CORE_LAZY_STREAM(stream, condition) \
  !(condition) ? (void)0 : ::core::LogMessageVoidify() & (stream)

#define CORE_EAT_STREAM_PARAMETERS(ignored) \
  true || (ignored)                        \
      ? (void)0                            \
      : ::core::LogMessageVoidify() &       \
            ::core::LogMessage(::Core::LOG_FATAL, 0, 0, nullptr).stream()

#define CORE_LOG_IS_ON(severity) \
  (::core::ShouldCreateLogMessage(::core::LOG_##severity))

#define CORE_LOG(severity) \
  CORE_LAZY_STREAM(CORE_LOG_STREAM(severity), CORE_LOG_IS_ON(severity))

#define CORE_CHECK(condition)                                              \
  CORE_LAZY_STREAM(                                                        \
      ::core::LogMessage(::core::LOG_FATAL, __FILE__, __LINE__, #condition) \
          .stream(),                                                      \
      !(condition))

#define CORE_VLOG_IS_ON(verbose_level) \
  ((verbose_level) <= ::core::GetVlogVerbosity())

// The VLOG macros log with negative verbosities.
#define CORE_VLOG_STREAM(verbose_level) \
  ::Core::LogMessage(-verbose_level, __FILE__, __LINE__, nullptr).stream()

#define CORE_VLOG(verbose_level) \
  CORE_LAZY_STREAM(CORE_VLOG_STREAM(verbose_level), CORE_VLOG_IS_ON(verbose_level))

#ifndef NDEBUG
#define CORE_DLOG(severity) CORE_LOG(severity)
#define CORE_DCHECK(condition) CORE_CHECK(condition)
#else
#define CORE_DLOG(severity) CORE_EAT_STREAM_PARAMETERS(true)
#define CORE_DCHECK(condition) CORE_EAT_STREAM_PARAMETERS(condition)
#endif

#define CORE_UNREACHABLE()                          \
  {                                                \
    CORE_LOG(ERROR) << "Reached unreachable code."; \
    ::Core::KillProcess();                          \
  }

#endif //TEST_LOGGING_H
