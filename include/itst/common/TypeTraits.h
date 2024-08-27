#pragma once

#include <type_traits>
#if __cplusplus < 202002L
#include "itst/common/detail/TypeTraitsCXX17.h"
#else
#include "itst/common/detail/TypeTraitsCXX20.h"
#endif

#include <optional>
#include <ostream>
#include <string>

namespace itst {

namespace detail {
// NOLINTNEXTLINE(readability-identifier-naming)
template <typename T> struct is_optional : std::false_type {};
template <typename T> struct is_optional<std::optional<T>> : std::true_type {};
} // namespace detail

// NOLINTNEXTLINE(readability-identifier-naming)
template <typename T> constexpr bool is_nothrow_to_string() noexcept {
  using std::to_string;
  return noexcept(to_string(std::declval<const T &>()));
}

template <typename T>
inline decltype(auto)
// NOLINTNEXTLINE(readability-identifier-naming)
adl_to_string(const T &value) noexcept(is_nothrow_to_string<T>()) {
  using std::to_string;
  return to_string(value);
}

template <typename... T> struct Overloaded : T... {
  using T::operator()...;
};
template <typename... T> Overloaded(T...) -> Overloaded<T...>;

template <typename T>
// NOLINTNEXTLINE(readability-identifier-naming)
static constexpr bool is_optional_v = detail::is_optional<T>::value;

/// Utility to check the typename of a template instantiation
template <typename Str> static void tell(Str /*S*/) noexcept {
  puts(__PRETTY_FUNCTION__);
}

template <typename T, typename Enable = void> struct LogTraits {
  // template <typename Printer>
  // static void printAccordingToType(const T &item, Printer printer);
};

template <typename T, typename Printer, typename = void>
// NOLINTNEXTLINE(readability-identifier-naming)
struct has_log_traits : std::false_type {};
template <typename T, typename Printer>
struct has_log_traits<T, Printer,
                      std::void_t<decltype(LogTraits<T>::printAccordingToType(
                          std::declval<const T &>(), std::declval<Printer>()))>>
    : std::true_type {};

template <typename T, typename Printer>
// NOLINTNEXTLINE(readability-identifier-naming)
static constexpr bool has_log_traits_v = has_log_traits<T, Printer>::value;

template <typename T> struct LogTraits<std::optional<T>> {
  template <typename Printer>
  static void printAccordingToType(
      const std::optional<T> &item,
      Printer printer) noexcept(Printer::template isPrintNoexcept<T>()) {
    if (!item) {
      printer("<none>");
    } else {
      printer("<some ");
      printer(*item);
      printer(">");
    }
  }
};

} // namespace itst
