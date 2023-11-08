#pragma once

#include "LoggerBase.h"

namespace itst {
class ITST_API StringLogger : public LoggerImpl<StringLogger> {
  friend LoggerImpl;

public:
  explicit StringLogger(std::string_view class_name,
                        LogSeverity sev = DefaultSeverity) noexcept;

  ~StringLogger();

  [[nodiscard]] std::string_view str() noexcept;

private:
  [[nodiscard]] FILE *getFileHandle() const noexcept { return file_handle; }

  FILE *file_handle{};
  char *data{};
  size_t size{};
};
} // namespace itst