#pragma once

#include "itst/Core.h"

// See LLVM ErrorHandling.h/.cpp

namespace itst {
[[noreturn]] void ITST_API unreachable_internal(const char *msg = nullptr,
                                                const char *file = nullptr,
                                                unsigned line = 0);
}

/// Same as llvm_unreachable

/// Marks that the current location is not supposed to be reachable.
/// In !NDEBUG builds, prints the message and location info to stderr.
/// In NDEBUG builds, if the platform does not support a builtin unreachable
/// then we call an internal LLVM runtime function. Otherwise the behavior is
/// controlled by the CMake flag
///   -DLLVM_UNREACHABLE_OPTIMIZE
/// * When "ON" (default) llvm_unreachable() becomes an optimizer hint
///   that the current location is not supposed to be reachable: the hint
///   turns such code path into undefined behavior.  On compilers that don't
///   support such hints, prints a reduced message instead and aborts the
///   program.
/// * When "OFF", a builtin_trap is emitted instead of an
//    optimizer hint or printing a reduced message.
///
/// Use this instead of assert(0). It conveys intent more clearly, suppresses
/// diagnostics for unreachable code paths, and allows compilers to omit
/// unnecessary code.
#ifndef NDEBUG
#define itst_unreachable(msg)                                                  \
  ::itst::unreachable_internal(msg, __FILE__, __LINE__)
#elif !defined(ITST_BUILTIN_UNREACHABLE)
#define itst_unreachable(msg) ::itst::unreachable_internal()
#elif LLVM_UNREACHABLE_OPTIMIZE
#define itst_unreachable(msg) LLVM_BUILTIN_UNREACHABLE
#else
#define itst_unreachable(msg)                                                  \
  do {                                                                         \
    ITST_BUILTIN_TRAP;                                                         \
    ITST_BUILTIN_UNREACHABLE;                                                  \
  } while (false)
#endif
