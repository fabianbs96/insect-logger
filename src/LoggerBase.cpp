#include "itst/LoggerBase.h"

#include "itst/LogSeverity.h"
#include "itst/common/TemplateString.h"

#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string_view>
#include <type_traits>

namespace itst {
std::optional<LogSeverity> LoggerBase::global_enforced_log_severity{};

#if defined(_GNU_SOURCE) && !defined(ITST_DISABLE_LOGGER)
auto LoggerBase::FileLock::create(FILE *file_handle) noexcept -> FileLock {
  FileLock lck;
  lck.file_handle = file_handle;
  if (file_handle)
    flockfile(file_handle);

  return lck;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LoggerBase::FileLock::destroy() noexcept {
  assert(file_handle != nullptr);
  funlockfile(file_handle);
}
#endif

void LoggerBase::FileWriter::operator()(
    std::string_view content) const noexcept {
#ifdef _GNU_SOURCE
  /// NOTE: fwrite_unlocked is non-standard, unfortunately. For performance
  /// reasons, call it whenever available
  fwrite_unlocked(content.data(), 1, content.size(), file_handle);
#else
  fwrite(content.data(), 1, content.size(), file_handle);
#endif
}

void LoggerBase::flushImpl(FILE *file_handle) noexcept { fflush(file_handle); }

#ifdef ITST_ENABLE_COLORS

// Note: partly copied from LLVM's raw_ostream impl
// color order matches ANSI escape sequence, don't change
enum class Colors : char {
  BLACK = 0,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  WHITE,
  SAVEDCOLOR,
  RESET,
};

// Colorcode macros copied from LLVM

#define COLOR(FGBG, CODE, BOLD) "\033[0;" BOLD FGBG CODE "m"

#define ALLCOLORS(FGBG, BOLD)                                                  \
  {                                                                            \
    COLOR(FGBG, "0", BOLD), COLOR(FGBG, "1", BOLD), COLOR(FGBG, "2", BOLD),    \
        COLOR(FGBG, "3", BOLD), COLOR(FGBG, "4", BOLD),                        \
        COLOR(FGBG, "5", BOLD), COLOR(FGBG, "6", BOLD), COLOR(FGBG, "7", BOLD) \
  }

static constexpr char ColorCodes[2][2][8][10] = {
    {ALLCOLORS("3", ""), ALLCOLORS("3", "1;")},
    {ALLCOLORS("4", ""), ALLCOLORS("4", "1;")}};

static constexpr const char *outputColor(Colors code, bool bold,
                                         bool bg) noexcept {
  // NOLINTNEXTLINE
  return ColorCodes[bg ? 1 : 0][bold ? 1 : 0][char(code) & 7];
}

template <Colors Code> static constexpr auto boldFwColor() noexcept {
  return ITST_STR(outputColor(Code, true, false));
}

static constexpr auto ResetColorCode = ITST_STR("\033[0m");
static constexpr auto BoldColorCode = ITST_STR("\033[0;1m");

template <LogSeverity Sev> static constexpr auto sevColorCode() noexcept {
  if constexpr (Sev == LogSeverity::Trace)
    return ITST_STR("");
  else if constexpr (Sev == LogSeverity::Debug)
    return BoldColorCode;
  else if constexpr (Sev == LogSeverity::Info)
    return boldFwColor<Colors::BLUE>();
  else if constexpr (Sev == LogSeverity::Warning)
    return boldFwColor<Colors::MAGENTA>();
  else
    return boldFwColor<Colors::RED>();
}
template <LogSeverity Sev>
static constexpr auto resetAndOpenBracket() noexcept {
  if constexpr (Sev == LogSeverity::Trace)
    return ITST_STR("[");
  else
    return ResetColorCode + ITST_STR("[");
}

template <LogSeverity Sev, typename FieldT>
static constexpr auto ColoredSeverityFieldImpl =
    ITST_STR("]") + sevColorCode<Sev>() + FieldT{} + resetAndOpenBracket<Sev>();

static inline std::string_view
coloredSeverityField(LogSeverity msg_sev) noexcept {
  static constexpr auto ResetAndOpenBracket = ResetColorCode + ITST_STR("[");
  switch (msg_sev) {
#define ITST_LOG_SEVERITY(NAME, REP)                                           \
  case LogSeverity::NAME:                                                      \
    static constexpr auto Str##NAME = ITST_STR("[" #REP "]");                  \
    return ColoredSeverityFieldImpl<LogSeverity::NAME, decltype(Str##NAME)>;
#include "itst/LogSeverity.def"
  }

  ITST_BUILTIN_UNREACHABLE;
}

#endif // ITST_ENABLE_COLORS

void LoggerBase::printHeader(std::string_view class_name, LogSeverity msg_sev,
                             FileWriter writer,
                             std::true_type /*with_colors*/) noexcept {
#ifndef ITST_ENABLE_COLORS
  return printHeader(msg_sev, writer, std::false_type{});
#else

  writer(boldFwColor<Colors::BLACK>() + ITST_STR("["));
  printTimestamp(writer);
  writer(coloredSeverityField(msg_sev));
  writer(class_name);
  writer("]: ");
#endif
}

static constexpr std::string_view severityField(LogSeverity msg_sev) noexcept {
  switch (msg_sev) {
#define ITST_LOG_SEVERITY(NAME, REP)                                           \
  case LogSeverity::NAME:                                                      \
    return "][" #REP "][";
#include "itst/LogSeverity.def"
  }

  ITST_BUILTIN_UNREACHABLE;
}

void LoggerBase::printHeader(std::string_view class_name, LogSeverity msg_sev,
                             FileWriter writer,
                             std::false_type /*with_colors*/) noexcept {
  writer("[");
  printTimestamp(writer);
  writer(severityField(msg_sev));
  writer(class_name);
  writer("]: ");
}

static constexpr std::array<char, 200> digits() noexcept {
  std::array<char, 200> ret{};

  for (size_t i = 0; i < 100; ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    ret[i * 2] = char('0' + i / 10);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    ret[i * 2 + 1] = char('0' + i % 10);
  }

  return ret;
}

void LoggerBase::printTimestamp(FileWriter writer) noexcept {
  static constexpr auto Digits = digits();

  struct timespec current_time {};
  clock_gettime(CLOCK_REALTIME, &current_time);

  struct tm current_local_time = {};
#ifdef _MSC_VER
  // For whatever reason the parameters on msvc are swapped
  localtime_s(&current_local_time, &current_time.tv_sec);
#else
  localtime_r(&current_time.tv_sec, &current_local_time);
#endif
  auto year = current_local_time.tm_year + 1900;
  auto month = current_local_time.tm_mon + 1;
  auto day = current_local_time.tm_mday;
  auto hour = current_local_time.tm_hour;
  auto minutes = current_local_time.tm_min;
  auto seconds = current_local_time.tm_sec;

  auto current_microseconds = current_time.tv_nsec / 1000;

  static constexpr auto Print2 = [](char *ptr, size_t num) noexcept {
    assert(num < 100);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    memcpy(ptr, &Digits[num * 2], 2);
    return ptr + 2; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  };
  static constexpr auto Print4 = [](char *ptr, size_t num) noexcept {
    assert(num < 10'000);
    auto hun = num / 100;
    auto rem = num % 100;

    Print2(ptr, hun);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return Print2(ptr + 2, rem);
  };

  std::array<char, getTimestepLength() + 1> buf{};
  char *ptr = buf.data();

  ptr = Print4(ptr, year);
  *ptr++ = '-'; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ptr = Print2(ptr, month);
  *ptr++ = '-'; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ptr = Print2(ptr, day);
  *ptr++ = ' '; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ptr = Print2(ptr, hour);
  *ptr++ = ':'; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ptr = Print2(ptr, minutes);
  *ptr++ = ':'; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ptr = Print2(ptr, seconds);
  *ptr++ = '.'; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ptr = Print2(ptr, current_microseconds / 10'000);
  ptr = Print4(ptr, current_microseconds % 10'000);

  assert(ptr <= buf.data() + buf.size());

  writer(std::string_view(buf.data(), ptr - buf.data()));
}

} // namespace itst
