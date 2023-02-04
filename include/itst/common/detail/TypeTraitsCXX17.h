#pragma once

#include <string>
#include <string_view>
#include <type_traits>

namespace itst {
namespace detail {
template <typename T> struct has_adl_to_string {
  template <typename TT = T, typename = decltype(std::string_view(
                                 to_string(std::declval<TT>())))>
  static std::true_type test(int);
  template <typename TT = T,
            typename = decltype(std::to_string(std::declval<TT>()))>
  static std::true_type test(long);
  template <typename TT = T> static std::false_type test(...);

  static constexpr bool value =
      std::is_same_v<std::true_type, decltype(test(0))>;
};

static_assert(has_adl_to_string<int>::value);
static_assert(!has_adl_to_string<std::ostream>::value);

template <typename T, typename = void> struct is_printable : std::false_type {};
template <typename T>
struct is_printable<T, std::void_t<decltype(std::declval<std::ostream &>()
                                            << std::declval<T>())>>
    : std::true_type {};

template <typename T, typename = std::string_view>
struct has_str : std::false_type {};
template <typename T>
struct has_str<T, decltype(std::string_view(std::declval<const T &>().str()))>
    : std::true_type {};

template <typename T, typename = std::string_view>
struct has_toString : std::false_type {};
template <typename T>
struct has_toString<T, decltype(std::string_view(
                           std::declval<const T &>().toString()))>
    : std::true_type {};

template <typename T, typename = void> struct is_iterable : std::false_type {};
template <typename T>
struct is_iterable<T, std::enable_if_t<std::is_array_v<T>>> : std::true_type {};
template <typename T>
struct is_iterable<T, std::void_t<decltype(std::declval<const T &>().begin()),
                                  decltype(std::declval<const T &>().end())>>
    : std::true_type {};
} // namespace detail

template <typename T>
static constexpr bool has_adl_to_string_v = detail::has_adl_to_string<T>::value;

template <typename T>
static constexpr bool is_printable_v = detail::is_printable<T>::value;

template <typename T>
static constexpr bool has_str_v = detail::has_str<T>::value;

template <typename T>
static constexpr bool has_toString_v = detail::has_toString<T>::value;

template <typename T>
static constexpr bool is_iterable_v = detail::is_iterable<T>::value;
} // namespace itst
