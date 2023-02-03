#include "itst/LogSeverity.h"

#include "itst/common/StringSwitch.h"
#include "itst/common/ErrorHandling.h"

namespace itst {
std::string_view to_string(LogSeverity sev) {
  switch (sev) {
#define ITST_LOG_SEVERITY(NAME, REP)                                           \
  case LogSeverity::NAME:                                                      \
    return #REP;
#include "itst/LogSeverity.def"
  }
  itst_unreachable("Invalid LogSeverity");
}

std::optional<LogSeverity> from_string(std::string_view str) {
  return itst::StringSwitch<LogSeverity>(str)
#define ITST_LOG_SEVERITY(NAME, REP) .Case(#REP, LogSeverity::NAME)
#include "itst/LogSeverity.def"
      .NoDefault();
}

} // namespace itst
