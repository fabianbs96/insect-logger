#pragma once

#include "LoggerBase.h"

namespace itst {
class ConsoleLogger : public LoggerBase {
public:
  constexpr ConsoleLogger(std::string_view class_name, LogSeverity sev =
#ifdef ITST_DEBUG_LOGGING
                                                           LogSeverity::Debug
#else
                                                           LogSeverity::Info
#endif
                          ) noexcept
      : LoggerBase(class_name, sev, StandardErr{}) {
  }
};
} // namespace itst