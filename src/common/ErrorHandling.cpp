#include "itst/common/ErrorHandling.h"

#include <array>
#include <charconv>
#include <cstdio>
#include <cstdlib>

// See LLVM ErrorHandling.cpp

void itst::unreachable_internal(const char *msg, const char *file,
                                unsigned line) {
  if (msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
  }

  fputs("UNREACHABLE executed", stderr);
  if (file) {
    fputs(" at ", stderr);
    fputs(file, stderr);
    fputc(':', stderr);

    std::array<char, 12> Buf;
    auto [ptr, err] = std::to_chars(Buf.data(), Buf.data() + Buf.size(), line);
    fwrite(Buf.data(), 1, std::distance(Buf.data(), ptr), stderr);
  }
  fputs("!\n", stderr);
  fflush(stderr);
  abort();
#ifdef ITST_BUILTIN_UNREACHABLE
  // Windows systems and possibly others don't declare abort() to be noreturn,
  // so use the unreachable builtin to avoid a Clang self-host warning.
  ITST_BUILTIN_UNREACHABLE;
#endif
}