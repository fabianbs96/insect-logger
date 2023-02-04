#include "itst/StringLogger.h"
#include "itst/Macros.h"
#include <cstdio>

namespace itst {
StringLogger::StringLogger(std::string_view class_name,
                           LogSeverity sev) noexcept
    : LoggerImpl(class_name, sev), file_handle(open_memstream(&data, &size)) {
#ifndef ITST_DISABLE_ASSERT
  if (!file_handle) {
    perror("Failed to open memorystream");
    ITST_BUILTIN_TRAP;
  }
#endif // ITST_DISABLE_ASSERT
}

StringLogger::~StringLogger() {
  if (file_handle) {
    fclose(file_handle);
  }
  if (data) {
    free(data);
  }
}

std::string_view StringLogger::str() noexcept {
  fflush_unlocked(file_handle);
  return std::string_view(data, size);
}
} // namespace itst