#pragma once

#define ITST_LOGGER static const ::itst::ConsoleLogger logger(__FUNCTION__)
#define ITST_LOGGER_SEV(SEV)                                                   \
  static const ::itst::ConsoleLogger logger(__FUNCTION__,                      \
                                            ::itst::LogSeverity::SEV)

#define ITST_LOGGER_CAT(CAT)                                                   \
  static const ::itst::ConsoleLogger logger { CAT }
#define ITST_LOGGER_CAT_SEV(CAT, SEV)                                          \
  static const ::itst::ConsoleLogger logger(CAT, ::itst::LogSeverity::SEV)

#define ITST_LOG(SEV, ...) logger.log(::itst::LogSeverity::SEV, __VA_ARGS__)
#define ITST_LOGF(SEV, FMT, ...)                                               \
  logger.logf(                                                                 \
      [] {                                                                     \
        struct Fmt {                                                           \
          const char *Data = FMT;                                              \
        };                                                                     \
        return Fmt{};                                                          \
      }(),                                                                     \
      ::itst::LogSeverity::SEV, __VA_ARGS__)

#define ITST_LOGGER_LOG(SEV, ...)                                              \
  {                                                                            \
    ITST_LOGGER;                                                               \
    ITST_LOG(SEV, __VA_ARGS__);                                                \
  }
#define ITST_LOGGER_LOGF(SEV, FMT, ...)                                        \
  {                                                                            \
    ITST_LOGGER;                                                               \
    ITST_LOGF(SEV, FMT, __VA_ARGS__);                                          \
  }
#define ITST_LOG_FLUSH() logger.flush()

// ---

#ifndef ITST_DISABLE_ASSERT
#define ITST_ASSERT(X, ...)                                                    \
  {                                                                            \
    if (!(X)) [[unlikely]] {                                                   \
      ITST_LOG(Fatal, __VA_ARGS__);                                            \
      ITST_LOG_FLUSH();                                                        \
      ITST_BUILTIN_TRAP;                                                       \
    }                                                                          \
  }
#define ITST_ASSERTF(X, FMT, ...)                                              \
  {                                                                            \
    if (!(X)) [[unlikely]] {                                                   \
      ITST_LOGF(Fatal, FMT, __VA_ARGS__);                                      \
      ITST_LOG_FLUSH();                                                        \
      ITST_BUILTIN_TRAP;                                                       \
    }                                                                          \
  }
#define ITST_LOGGER_ASSERT(X, ...)                                             \
  {                                                                            \
    if (!(X)) [[unlikely]] {                                                   \
      ITST_LOGGER_LOG(Fatal, __VA_ARGS__);                                     \
      ITST_LOG_FLUSH();                                                        \
      ITST_BUILTIN_TRAP;                                                       \
    }                                                                          \
  }
#define ITST_LOGGER_ASSERTF(X, ...)                                            \
  {                                                                            \
    if (!(X)) [[unlikely]] {                                                   \
      ITST_LOGGER_LOGF(Fatal, FMT, __VA_ARGS__);                               \
      ITST_LOG_FLUSH();                                                        \
      ITST_BUILTIN_TRAP;                                                       \
    }                                                                          \
  }

#else

#define ITST_ASSERT(X, ...)                                                    \
  {}
#define ITST_ASSERTF(X, FMT, ...)                                              \
  {}
#define ITST_LOGGER_ASSERT(X, ...)                                             \
  {}
#define ITST_LOGGER_ASSERTF(X, ...)                                            \
  {}

#endif // ITST_DISABLE_ASSERT