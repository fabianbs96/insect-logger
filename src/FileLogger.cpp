#include "itst/FileLogger.h"
#include "itst/Core.h"
#include "itst/LoggerBase.h"

#include <cstdio>

namespace itst {
FileLogger::FileLogger(const char *file_name, std::string_view class_name,
                       LogSeverity sev) noexcept
    : LoggerImpl(class_name, sev), file_handle(fopen(file_name, "a+")) {
#ifndef ITST_DISABLE_ASSERT
  if (!file_handle) {
    perror("Failed to open file stream");
    ITST_BUILTIN_TRAP;
  }
#endif // ITST_DISABLE_ASSERT
}

FileLogger::~FileLogger() {
  if (file_handle) {
    fclose(file_handle);
  }
}
} // namespace itst