#pragma once

#include "itst/Core.h"

#include <optional>
#include <string_view>

namespace itst {

// Severity of a log entry.
enum class LogSeverity {
#define ITST_LOG_SEVERITY(NAME, REP) NAME,
#include "itst/LogSeverity.def"
};

std::string_view ITST_API to_string(LogSeverity sev) noexcept;
std::optional<LogSeverity> ITST_API from_string(std::string_view str) noexcept;

} // namespace itst
