#pragma once

#include "LoggerBase.h"

namespace itst {
class ConsoleLogger : public LoggerImpl<ConsoleLogger> {
  friend LoggerImpl;

public:
  constexpr ConsoleLogger(std::string_view class_name,
                          LogSeverity sev = DefaultSeverity) noexcept
      : LoggerImpl(class_name, sev) {}

private:
  [[nodiscard]] FILE *getFileHandle() const noexcept { return stderr; }
};
} // namespace itst