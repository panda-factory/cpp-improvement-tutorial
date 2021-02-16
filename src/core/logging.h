//
// Created by admin on 2021/1/28.
//

#ifndef TEST_LOGGING_H
#define TEST_LOGGING_H
#include <sstream>

#include "log_level.h"
#include "macros.h"

namespace wtf {

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

    WTF_DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

int GetVlogVerbosity();

bool ShouldCreateLogMessage(LogSeverity severity);

[[noreturn]] void KillProcess();

} // namespace wtf

#define WTF_LOG_STREAM(severity) \
  ::wtf::LogMessage(::wtf::LOG_##severity, __FILE__, __LINE__, nullptr).stream()

#define WTF_LAZY_STREAM(stream, condition) \
  !(condition) ? (void)0 : ::wtf::LogMessageVoidify() & (stream)

#define WTF_EAT_STREAM_PARAMETERS(ignored) \
  true || (ignored)                        \
      ? (void)0                            \
      : ::wtf::LogMessageVoidify() &       \
            ::wtf::LogMessage(::wtf::LOG_FATAL, 0, 0, nullptr).stream()

#define WTF_LOG_IS_ON(severity) \
  (::wtf::ShouldCreateLogMessage(::wtf::LOG_##severity))

#define WTF_LOG(severity) \
  WTF_LAZY_STREAM(WTF_LOG_STREAM(severity), WTF_LOG_IS_ON(severity))

#define WTF_CHECK(condition)                                              \
  WTF_LAZY_STREAM(                                                        \
      ::wtf::LogMessage(::wtf::LOG_FATAL, __FILE__, __LINE__, #condition) \
          .stream(),                                                      \
      !(condition))

#define WTF_VLOG_IS_ON(verbose_level) \
  ((verbose_level) <= ::wtf::GetVlogVerbosity())

// The VLOG macros log with negative verbosities.
#define WTF_VLOG_STREAM(verbose_level) \
  ::wtf::LogMessage(-verbose_level, __FILE__, __LINE__, nullptr).stream()

#define CORE_VLOG(verbose_level) \
  WTF_LAZY_STREAM(WTF_VLOG_STREAM(verbose_level), WTF_VLOG_IS_ON(verbose_level))

#ifndef NDEBUG
#define WTF_DLOG(severity) WTF_LOG(severity)
#define WTF_DCHECK(condition) WTF_CHECK(condition)
#else
#define WTF_DLOG(severity) WTF_EAT_STREAM_PARAMETERS(true)
#define WTF_DCHECK(condition) WTF_EAT_STREAM_PARAMETERS(condition)
#endif

#define WTF_UNREACHABLE()                          \
  {                                                \
    WTF_LOG(ERROR) << "Reached unreachable code."; \
    ::wtf::KillProcess();                          \
  }

#endif //TEST_LOGGING_H
