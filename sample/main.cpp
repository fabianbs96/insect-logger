
#include "itst/ConsoleLogger.h"
#include "itst/Macros.h"

int main() {
  static constexpr itst::ConsoleLogger logger{"main"};

  struct Foo {
    std::string str() const { return "Hello"; }
  };

  logger.logInfo("Dies ist ein Test ", 42, Foo{});

  ITST_LOGF(Warning, "Testformat{} {}.{}", 42, Foo{}, "123");

  ITST_LOGGER_LOGF(Warning, "Testformat{} {}.{}", 42, Foo{}, "123");
  ITST_LOGF(Info, "Testescape }}{{");
  ITST_LOGF(Info, "Testfmtescape {{{}}}", Foo{});

  std::vector<std::vector<int>> vec = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  ITST_LOG(Info, vec);

  ITST_ASSERT(vec.size() == 3, "Invalid size");
  ITST_ASSERTF(vec.size() == 3, "Invalid size");

  logger.stream(itst::LogSeverity::Info) << Foo{} << " from"
                                         << " stream";
  logger.stream(itst::LogSeverity::Info) << "> " << 42 << "blah";
  ITST_LOG_STREAM(Info) << Foo{} << " from stream macro";
}
