#include "itst/LoggerBase.h"

#include <array>
#include <cassert>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>

namespace itst {
std::optional<LogSeverity> LoggerBase::global_enforced_log_severity{};

#if defined(_GNU_SOURCE) && !defined(ITST_DISABLE_LOGGER)
auto LoggerBase::FileLock::create(FILE *file_handle) noexcept -> FileLock {
  FileLock Lck;
  Lck.file_handle = file_handle;
  flockfile(file_handle);
  return Lck;
}

void LoggerBase::FileLock::destroy() noexcept { funlockfile(file_handle); }
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

void LoggerBase::printHeader(LogSeverity msg_sev,
                             FileWriter writer) const noexcept {
  writer("[");
  printTimestamp(writer);
  writer("][");
  writer(to_string(msg_sev));
  writer("][");
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

  auto print2 = [](char *ptr, auto num) {
    assert(num < 100);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    memcpy(ptr, &Digits[num * 2], 2);
    return ptr + 2; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  };
  auto print4 = [print2](char *ptr, auto num) {
    assert(num < 10'000);
    print2(ptr, num / 100);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return print2(ptr + 2, num % 100);
  };

  std::array<char, getTimestepLength() + 1> buf{};
  char *ptr = buf.data();

  ptr = print4(ptr, year);
  *ptr++ = '-'; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ptr = print2(ptr, month);
  *ptr++ = '-'; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ptr = print2(ptr, day);
  *ptr++ = ' '; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ptr = print2(ptr, hour);
  *ptr++ = ':'; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ptr = print2(ptr, minutes);
  *ptr++ = ':'; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ptr = print2(ptr, seconds);
  *ptr++ = '.'; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ptr = print2(ptr, current_microseconds / 10'000);
  ptr = print4(ptr, current_microseconds % 10'000);

  assert(ptr <= buf.data() + buf.size());

  writer(std::string_view(buf.data(), ptr - buf.data()));
}

} // namespace itst
