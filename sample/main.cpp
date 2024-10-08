
#include "itst/ConsoleLogger.h"
#include "itst/LoggerBase.h"
#include "itst/Macros.h"

#include <iostream>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

template <typename Fmt> void bar(Fmt /*F*/) {
  using namespace itst;
  constexpr auto Splits =
      cxx17::splitFormatString(cxx17::appendLf(cxx17::getCStr<Fmt>()));

  std::apply(
      [](auto... str) { (((std::cout << "> " << str.str() << '\n')), ...); },
      Splits);
}

void foo() {
  bar(ITST_FMT("Testescape }}{{"));
  bar(ITST_FMT("Testfmtescape {{{}}}"));
}

int main() {
  static constexpr itst::ConsoleLogger logger{"main"};

  struct Foo {
    [[nodiscard]] std::string str() const { return "Hello"; }
  };

  logger.logInfo("Dies ist ein Test ", 42, Foo{});

  ITST_LOGF(Warning, "Testformat{} {}.{}", 42, Foo{}, "123");

  ITST_LOGGER_LOGF(Warning, "Testformat{} {}.{} {}", 42, Foo{}, "123",
                   std::optional<int>{});
  ITST_LOGF(Info, "Testescape }}{{");
  ITST_LOGF(Info, "Testfmtescape {{{}}}", Foo{});

  std::vector<std::vector<int>> vec = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  ITST_LOG(Info, vec);
  ITST_LOG(Info, std::vector<std::optional<std::vector<int>>>{
                     std::nullopt, {{42, 43}}, std::nullopt});

  ITST_ASSERT(vec.size() == 3, "Invalid size");
  ITST_ASSERTF(vec.size() == 3, "Invalid size");

  logger.stream(itst::LogSeverity::Info) << Foo{} << " from" << " stream";
  logger.stream(itst::LogSeverity::Info) << "> " << 42 << "blah";
  ITST_LOG_STREAM(Info) << Foo{} << " from stream macro, " << 0.3 << " "
                        << 42.4222333L;

  foo();
}
