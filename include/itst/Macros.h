#pragma once

#include "itst/LogSeverity.h"
#include "itst/LoggerBase.h"

#define ITST_LOGGER static constexpr ::itst::ConsoleLogger logger(__FUNCTION__)
#define ITST_LOGGER_SEV(SEV)                                                   \
  static const ::itst::ConsoleLogger logger(__FUNCTION__,                      \
                                            ::itst::LogSeverity::SEV)

#define ITST_LOGGER_CAT(CAT)                                                   \
  static constexpr ::itst::ConsoleLogger logger { CAT }
#define ITST_LOGGER_CAT_SEV(CAT, SEV)                                          \
  static constexpr ::itst::ConsoleLogger logger(CAT, ::itst::LogSeverity::SEV)

#define ITST_LOG(SEV, ...) logger.log(::itst::LogSeverity::SEV, __VA_ARGS__)

#define ITST_FMT(FMT)                                                          \
  [] {                                                                         \
    struct Fmt {                                                               \
      const char *Data = FMT;                                                  \
    };                                                                         \
    return Fmt{};                                                              \
  }()
#define ITST_LOGF(SEV, FMT, ...)                                               \
  logger.logf(ITST_FMT(FMT), ::itst::LogSeverity::SEV, ##__VA_ARGS__)

#define ITST_LOGGER_LOG(SEV, ...)                                              \
  do {                                                                         \
    ITST_LOGGER;                                                               \
    ITST_LOG(SEV, __VA_ARGS__);                                                \
  } while (false)
#define ITST_LOGGER_LOGF(SEV, FMT, ...)                                        \
  do {                                                                         \
    ITST_LOGGER;                                                               \
    ITST_LOGF(SEV, FMT, ##__VA_ARGS__);                                        \
  } while (false)
#define ITST_LOG_FLUSH() logger.flush()

#define ITST_LOG_STREAM(SEV) logger.stream(::itst::LogSeverity::SEV)
// ---

#ifndef ITST_DISABLE_ASSERT

namespace itst::detail {
template <typename LoggerT, typename... Msg>
static inline void assertFailMessage(const LoggerImpl<LoggerT> &Logger,
                                     const char *File, unsigned Line,
                                     const Msg &...M) {
  if constexpr (sizeof...(Msg) != 0) {
    Logger.log(LogSeverity::Fatal, File, ":", Line, ": note: ", M...);
  }
}

template <typename LoggerT, typename Fmt, typename... Msg>
static inline void assertFailMessagef(const LoggerImpl<LoggerT> &Logger, Fmt F,
                                      const char *File, unsigned Line,
                                      const Msg &...M) {
  if constexpr (sizeof...(Msg) != 0) {
    Logger.logf(F, LogSeverity::Fatal, File, Line, M...);
  }
}
} // namespace itst::detail

#define ITST_ASSERT(X, ...)                                                    \
  do {                                                                         \
    if (!(X)) [[unlikely]] {                                                   \
      ITST_LOG(Fatal, __FILE__, ":", __LINE__, ": Assertion failed: ", #X);    \
      ::itst::detail::assertFailMessage(logger, __FILE__, __LINE__,            \
                                        ##__VA_ARGS__);                        \
      ITST_LOG_FLUSH();                                                        \
      ITST_ABORT;                                                              \
    }                                                                          \
  } while (false)
#define ITST_ASSERTF(X, FMT, ...)                                              \
  do {                                                                         \
    if (!(X)) [[unlikely]] {                                                   \
      ITST_LOG(Fatal, __FILE__, ":", __LINE__, ": Assertion failed: ", #X);    \
      ::itst::detail::assertFailMessagef(logger,                               \
                                         ITST_FMT("{}:{}: note: " FMT),        \
                                         __FILE__, __LINE__, ##__VA_ARGS__);   \
      ITST_LOG_FLUSH();                                                        \
      ITST_ABORT;                                                              \
    }                                                                          \
  } while (false)
#define ITST_LOGGER_ASSERT(X, ...)                                             \
  do {                                                                         \
    ITST_LOGGER;                                                               \
    ITST_ASSERT(X, __VA_ARGS__);                                               \
  } while (false)

#define ITST_LOGGER_ASSERTF(X, FMT, ...)                                       \
  do {                                                                         \
    ITST_LOGGER;                                                               \
    ITST_ASSERTF(X, FMT, ##__VA_ARGS__);                                       \
  } while (false)

#else // ITST_DISABLE_ASSERT

#define ITST_ASSERT(X, ...)                                                    \
  do {                                                                         \
  } while (false)
#define ITST_ASSERTF(X, FMT, ...)                                              \
  do {                                                                         \
  } while (false)
#define ITST_LOGGER_ASSERT(X, ...)                                             \
  do {                                                                         \
  } while (false)
#define ITST_LOGGER_ASSERTF(, FMT, ...)                                        \
  do {                                                                         \
  } while (false)

#endif // ITST_DISABLE_ASSERT