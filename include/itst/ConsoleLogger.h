#pragma once

#include "LoggerBase.h"

#include <type_traits>


#ifndef ITST_CONSOLE_LOGGER_TARGET
#define ITST_CONSOLE_LOGGER_TARGET stderr
#endif

namespace itst {
class ConsoleLogger : public LoggerImpl<ConsoleLogger> {
  friend LoggerImpl;

public:
  constexpr ConsoleLogger(std::string_view class_name,
                          LogSeverity sev = DefaultSeverity) noexcept
      : LoggerImpl(class_name, sev) {}

private:
  [[nodiscard]] FILE *getFileHandle() const noexcept {
    return ITST_CONSOLE_LOGGER_TARGET;
  }

  [[nodiscard]] constexpr std::true_type withColors() const noexcept {
    return {};
  }
};
} // namespace itst
