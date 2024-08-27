#pragma once

#include <concepts>
#include <ostream>

namespace itst {

template <typename T>
concept has_adl_to_string_v = requires(const T &val) {
  { to_string(val) } -> std::convertible_to<std::string_view>;
} || requires(const T &val) {
  { std::to_string(val) } -> std::convertible_to<std::string_view>;
};

template <typename T>
concept is_printable_v = requires(std::ostream &os, const T &val) {
  { os << val };
};

template <typename T>
concept has_str_v = requires(const T &val) {
  { val.str() } -> std::convertible_to<std::string_view>;
};

template <typename T>
concept has_toString_v = requires(const T &val) {
  { val.toString() } -> std::convertible_to<std::string_view>;
};
template <typename T>
concept is_iterable_v = std::is_array_v<T> || requires(const T &val) {
  { val.begin() };
  { val.end() };
};

} // namespace itst