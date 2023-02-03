#pragma once

#include <ostream>

namespace itst {

template <typename T>
concept has_adl_to_string_v = requires(const T &Val) {
  { to_string(Val) } -> std::convertible_to<std::string_view>;
}
|| requires(const T &Val) {
  { std::to_string(Val) } -> std::convertible_to<std::string_view>;
};

template <typename T>
concept is_printable_v = requires(std::ostream &OS, const T &Val) {
  {OS << Val};
};

template <typename T>
concept has_str_v = requires(const T &Val) {
  { Val.str() } -> std::convertible_to<std::string_view>;
};

template <typename T>
concept has_toString_v = requires(const T &Val) {
  { Val.toString() } -> std::convertible_to<std::string_view>;
};
template <typename T>
concept is_iterable_v = std::is_array_v<T> || requires(const T &Val) {
  {Val.begin()};
  {Val.end()};
};

} // namespace itst