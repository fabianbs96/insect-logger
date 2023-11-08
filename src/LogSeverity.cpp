#include "itst/LogSeverity.h"

#include "itst/common/ErrorHandling.h"
#include "itst/common/StringSwitch.h"

namespace itst {
std::string_view to_string(LogSeverity sev) noexcept {
  switch (sev) {
#define ITST_LOG_SEVERITY(NAME, REP)                                           \
  case LogSeverity::NAME:                                                      \
    return #REP;
#include "itst/LogSeverity.def"
  }
  itst_unreachable("Invalid LogSeverity");
}

std::optional<LogSeverity> from_string(std::string_view str) noexcept {
  return itst::StringSwitch<LogSeverity>(str)
#define ITST_LOG_SEVERITY(NAME, REP) .Case(#REP, LogSeverity::NAME)
#include "itst/LogSeverity.def"
      .NoDefault();
}

} // namespace itst
