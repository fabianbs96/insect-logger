#pragma once

#include "LoggerBase.h"

namespace itst {
class FileLogger : public LoggerImpl<FileLogger> {
public:
  explicit FileLogger(const char *file_name, std::string_view class_name,
                      LogSeverity sev = DefaultSeverity) noexcept;
  explicit FileLogger(const std::string &file_name, std::string_view class_name,
                      LogSeverity sev = DefaultSeverity) noexcept
      : FileLogger(file_name.c_str(), class_name, sev) {}
  ~FileLogger();

  [[nodiscard]] FILE *getFileHandle() const noexcept { return file_handle; }

private:
  FILE *file_handle{};
};
} // namespace itst