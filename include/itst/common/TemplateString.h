#pragma once

#include <algorithm>
#include <array>
#include <string_view>
#include <tuple>

namespace itst {

namespace cxx17 {
template <char... Str> struct TemplateString {
  [[nodiscard]] static constexpr size_t size() noexcept {
    return sizeof...(Str);
  }
  [[nodiscard]] static constexpr bool empty() noexcept { return size() == 0; }

  static constexpr char Data[] = {Str..., '\0'};

  [[nodiscard]] static constexpr std::string_view str() noexcept {
    return Data;
  }

  template <char C> using append_t = TemplateString<Str..., C>;
};

template <typename T, typename U> struct tuple_append {};
template <typename U, typename... Ts>
struct tuple_append<std::tuple<Ts...>, U> {
  using type = std::tuple<Ts..., U>;
};
template <typename T, typename U>
using tuple_append_t = typename tuple_append<T, U>::type;

static constexpr size_t c_strlen(const char *Str, size_t Len = 0) {
  return *Str == '\0' ? Len : c_strlen(Str + 1, Len + 1);
}

template <typename StringProvider, size_t Len, char... Str>
struct TemplateStringBuilder {
  using type = typename TemplateStringBuilder<
      StringProvider, Len - 1, StringProvider{}.Data[Len - 1], Str...>::type;
};

template <typename StringProvider, char... Str>
struct TemplateStringBuilder<StringProvider, 0, Str...> {
  using type = TemplateString<Str...>;
};

template <typename StringProvider>
using CStrType =
    typename TemplateStringBuilder<StringProvider,
                                   c_strlen(StringProvider{}.Data)>::type;
template <typename StringProvider>
static constexpr CStrType<StringProvider> getCStr(StringProvider /*SP*/ = {}) {
  return {};
}

template <char... Str>
static constexpr TemplateString<Str..., '\n'>
appendLf(TemplateString<Str...> /*S*/) {
  return {};
}

// ---

template <typename Head, typename Tail, char... Str> struct FmtStringSplitter;

template <typename Head, typename Tail, char C, char... Str>
struct FmtStringSplitter<Head, Tail, C, Str...> {
  static_assert(C != '{' && C != '}',
                "The format string contains an invalid nested brace. Use brace "
                "escaping by doubling the brace you want to print instead.");
  using type = typename FmtStringSplitter<typename Head::template append_t<C>,
                                          Tail, Str...>::type;
};
template <typename Head, typename Tail, char... Str>
struct FmtStringSplitter<Head, Tail, '{', '}', Str...> {
  using type =
      typename FmtStringSplitter<TemplateString<>, tuple_append_t<Tail, Head>,
                                 Str...>::type;
};

template <typename Head, typename Tail, char... Str>
struct FmtStringSplitter<Head, Tail, '{', '{', Str...> {
  using type = typename FmtStringSplitter<typename Head::template append_t<'{'>,
                                          Tail, Str...>::type;
};

template <typename Head, typename Tail, char... Str>
struct FmtStringSplitter<Head, Tail, '}', '}', Str...> {
  using type = typename FmtStringSplitter<typename Head::template append_t<'}'>,
                                          Tail, Str...>::type;
};

template <typename Head, typename Tail> struct FmtStringSplitter<Head, Tail> {
  using type = tuple_append_t<Tail, Head>;
};

template <char... Str>
constexpr auto splitFormatString(TemplateString<Str...> S) noexcept {
  return typename FmtStringSplitter<TemplateString<>, std::tuple<>,
                                    Str...>::type{};
}
} // namespace cxx17

} // namespace itst