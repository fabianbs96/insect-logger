#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <optional>
#include <string_view>
#include <type_traits>

namespace itst {

// NOLINTNEXTLINE(readability-identifier-naming)
constexpr bool
any_of(std::string_view search_for,
       std::initializer_list<std::string_view> strings) noexcept {
  return std::any_of(strings.begin(), strings.end(),
                     [search_for](auto curr) { return search_for == curr; });
}

template <typename callable, typename returntype>
static constexpr bool invocable_with_return =
    std::is_invocable_r_v<returntype, callable>;

template <typename T = void> class StringSwitch {
public:
  inline constexpr explicit StringSwitch(
      std::string_view current_value) noexcept
      : current_value(current_value) {}
  StringSwitch(const StringSwitch &) = delete;
  StringSwitch(StringSwitch &&) = delete;

  StringSwitch &operator=(const StringSwitch &) = delete;
  StringSwitch &operator=(StringSwitch &&) = delete;

  template <typename Callback,
            typename = std::enable_if_t<invocable_with_return<Callback, T>>>
  [[nodiscard]] inline constexpr StringSwitch &&
  // NOLINTNEXTLINE(readability-identifier-naming)
  Case(std::initializer_list<std::string_view> cases, Callback &&cb) &&noexcept(
      std::is_nothrow_invocable_v<std::decay_t<Callback>>) {
    if (!result && any_of(current_value, cases)) {
      if constexpr (std::is_void_v<T>) {
        std::invoke(std::forward<Callback>(cb));
        result = true;
      } else {
        result = std::invoke(std::forward<Callback>(cb));
      }
    }
    return std::move(*this);
  }

  template <typename Callback,
            typename = std::enable_if_t<invocable_with_return<Callback, T>>>
  [[nodiscard]] inline constexpr StringSwitch &&
  // NOLINTNEXTLINE(readability-identifier-naming)
  Case(std::string_view sv, Callback &&cb) &&noexcept(
      std::is_nothrow_invocable_v<std::decay_t<Callback>>) {
    if (!result && sv == current_value) {
      if constexpr (std::is_void_v<T>) {
        std::invoke(std::forward<Callback>(cb));
        result = true;
      } else {
        result = std::invoke(std::forward<Callback>(cb));
      }
    }
    return std::move(*this);
  }

  template <typename TT = T>
  [[nodiscard]] inline constexpr std::enable_if_t<
      !std::is_void_v<TT> && std::is_convertible_v<TT, T>, StringSwitch &&>
  // NOLINTNEXTLINE(readability-identifier-naming)
  Case(std::initializer_list<std::string_view> cases,
       TT &&ret) &&noexcept(noexcept(T(std::forward<TT>(ret)))) {
    if (!result && any_of(current_value, cases)) {
      result = std::forward<TT>(ret);
    }
    return std::move(*this);
  }

  template <typename TT = T>
  // NOLINTNEXTLINE(readability-identifier-naming)
  [[nodiscard]] inline constexpr std::enable_if_t<
      !std::is_void_v<TT> && std::is_convertible_v<TT, T>, StringSwitch &&>
  Case(std::string_view sv,
       TT &&ret) &&noexcept(noexcept(T(std::forward<TT>(ret)))) {
    if (!result && sv == current_value) {
      result = std::forward<TT>(ret);
    }
    return std::move(*this);
  }

  template <typename Callback,
            typename = std::enable_if_t<invocable_with_return<Callback, T>>>
  // NOLINTNEXTLINE(readability-identifier-naming)
  inline constexpr T Default(Callback &&cb) &&noexcept(
      std::is_nothrow_invocable_v<std::decay_t<Callback>>
          &&std::is_nothrow_move_constructible_v<T>) {
    if constexpr (std::is_void_v<T>) {
      if (!result) {
        std::invoke(std::forward<Callback>(cb));
      }
    } else {
      if (!result) {
        return std::invoke(std::forward<Callback>(cb));
      }
      return std::move(*result);
    }
  }

  template <typename TT = T>
  [[nodiscard]] inline constexpr std::enable_if_t<
      !std::is_void_v<TT> && std::is_convertible_v<TT, T>, T>
  // NOLINTNEXTLINE(readability-identifier-naming)
  Default(TT &&ret) &&noexcept(noexcept(T(std::forward<TT>(ret))) &&
                               std::is_nothrow_move_constructible_v<T>) {
    if (result) {
      return std::move(*result);
    }
    return std::forward<TT>(ret);
  }

  /// Marker to show that we have no default case
  template <typename TT = T>
  // NOLINTNEXTLINE(readability-identifier-naming)
  inline constexpr std::enable_if_t<std::is_void_v<TT>> NoDefault() &&noexcept {
  }

  template <typename TT = T, typename = std::enable_if_t<!std::is_void_v<TT>>>
  [[nodiscard]] inline constexpr std::optional<T>
  // NOLINTNEXTLINE(readability-identifier-naming)
  NoDefault() &&noexcept(
      std::is_nothrow_move_constructible_v<std::optional<T>>) {
    return std::move(result);
  }

private:
  std::string_view current_value;
  std::conditional_t<std::is_void_v<T>, bool, std::optional<T>> result{};
};

} // namespace itst
