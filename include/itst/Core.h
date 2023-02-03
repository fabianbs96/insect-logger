#pragma once

#if defined(_WIN64) || defined(_Win32)

#ifdef ITST_BUILD_DLL
#define ITST_API __declspec(dllexport)
#else
#define ITST_API __declspec(dllimport)
#endif

#else
#define ITST_API
#endif

//
// See LLVM Compiler.h
//

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_unreachable) || defined(__GNUC__)
#define ITST_BUILTIN_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
#define ITST_BUILTIN_UNREACHABLE __assume(false)
#endif

#if __has_builtin(__builtin_trap) || defined(__GNUC__)
#define ITST_BUILTIN_TRAP __builtin_trap()
#elif defined(_MSC_VER)
// The __debugbreak intrinsic is supported by MSVC, does not require forward
// declarations involving platform-specific typedefs (unlike RaiseException),
// results in a call to vectored exception handlers, and encodes to a short
// instruction that still causes the trapping behavior we want.
#define ITST_BUILTIN_TRAP __debugbreak()
#else
#define ITST_BUILTIN_TRAP *(volatile int *)0x11 = 0
#endif
