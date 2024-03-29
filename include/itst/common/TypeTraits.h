#pragma once

#include <type_traits>
#if __cplusplus < 202002L
#include "itst/common/detail/TypeTraitsCXX17.h"
#else
#include "itst/common/detail/TypeTraitsCXX20.h"
#endif

#include <concepts>
#include <optional>
#include <ostream>
#include <string>

namespace itst {

namespace detail {
template <typename T> struct is_optional : std::false_type {};
template <typename T> struct is_optional<std::optional<T>> : std::true_type {};
} // namespace detail

template <typename T> constexpr bool is_nothrow_to_string() noexcept {
  using std::to_string;
  return noexcept(to_string(std::declval<const T &>()));
}

template <typename T>
inline decltype(auto)
adl_to_string(const T &value) noexcept(is_nothrow_to_string<T>()) {
  using std::to_string;
  return to_string(value);
}

template <typename... T> struct Overloaded : T... {
  using T::operator()...;
};
template <typename... T> Overloaded(T...) -> Overloaded<T...>;

template <typename T>
static constexpr bool is_optional_v = detail::is_optional<T>::value;

/// Utility to check the typename of a template instantiation
template <typename Str> static void tell(Str S) noexcept {
  puts(__PRETTY_FUNCTION__);
}

} // namespace itst
