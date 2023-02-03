#pragma once

#include "itst/Core.h"
#include "itst/LogSeverity.h"
#include "itst/common/TemplateString.h"
#include "itst/common/TypeTraits.h"

#include <cassert>
#include <cstdio>
#include <optional>
#include <sstream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace itst {

struct StandardOut {};
struct StandardErr {};

struct StreamType : public std::variant<StandardErr, StandardOut, FILE *> {
  using std::variant<StandardErr, StandardOut, FILE *>::variant;

  operator FILE *() const noexcept {
    return std::visit(Overloaded{[](StandardOut) { return stdout; },
                                 [](StandardErr) { return stderr; },
                                 [](FILE *F) { return F; }},
                      *this);
  }
};

/// An efficient, lightweight and thread-safe logger.
/// The only thing not thread-safe is global_enforced_log_severity; however,
/// it is expected to be set once at the beginning and never changed again.
class ITST_API LoggerBase {
  struct ITST_API FileWriter {
    FILE *file_handle{};
    void operator()(std::string_view content) const noexcept;
  };

  struct ITST_API LockedFileWriter : FileWriter {
#ifdef _GNU_SOURCE
    LockedFileWriter(FILE *file_handle) noexcept;
    ~LockedFileWriter() noexcept;
#endif
  };

public:
  static std::optional<LogSeverity> global_enforced_log_severity;

  explicit constexpr LoggerBase(std::string_view class_name, LogSeverity sev,
                                StreamType log_target = StandardErr{}) noexcept
      : file_handle(log_target), class_name(class_name), severity(sev) {}

  template <typename... Ts>
  const LoggerBase &log(LogSeverity msg_sev, const Ts &...log_items) const {
#ifndef ITST_DISABLE_LOGGER
    auto actual_sev = global_enforced_log_severity.value_or(severity);
    if (actual_sev > msg_sev) {
      return *this;
    }

    LockedFileWriter writer{file_handle};

    printHeader(msg_sev, writer);

    (printAccordingToType(log_items, writer), ...);

    writer("\n");
    // std::string_view content = "\n";
    // fwrite(content.data(), 1, content.size(), file_handle);

#ifdef ITST_DEBUG_LOGGING
    flush();
#endif

#endif
    return *this;
  }

  template <typename FormatStringProvider, typename... Ts>
  const LoggerBase &logf(FormatStringProvider /*FSP*/, LogSeverity msg_sev,
                         const Ts &...log_items) const {
    return internalLogf<FormatStringProvider>(
        msg_sev, std::tie(log_items...),
        std::make_index_sequence<sizeof...(Ts)>());
  }

  template <typename... Ts>
  const LoggerBase &logTrace(const Ts &...log_items) const {
    return log(LogSeverity::Trace, log_items...);
  }
  template <typename... Ts>
  const LoggerBase &logDebug(const Ts &...log_items) const {
    return log(LogSeverity::Debug, log_items...);
  }
  template <typename... Ts>
  const LoggerBase &logInfo(const Ts &...log_items) const {
    return log(LogSeverity::Info, log_items...);
  }
  template <typename... Ts>
  const LoggerBase &logWarning(const Ts &...log_items) const {
    return log(LogSeverity::Warning, log_items...);
  }
  template <typename... Ts>
  const LoggerBase &logError(const Ts &...log_items) const {
    return log(LogSeverity::Error, log_items...);
  }
  template <typename... Ts>
  const LoggerBase &logFatal(const Ts &...log_items) const {
    return log(LogSeverity::Fatal, log_items...);
  }

  void flush() const;

  // template <typename T> static std::string log_string(const T &item) { //
  // NOLINT
  //  std::string ret;
  //  llvm::raw_string_ostream ros(ret);
  //  printAccordingToType(item, [&ros](std::string_view str) { ros << str;
  //  }); return ret;
  //}

  static constexpr size_t getTimestepLength() noexcept {
    return sizeof("2022-11-02 15:10:22.633977") - 1;
  }

private:
  template <typename Str> static void tell(Str S) { puts(__PRETTY_FUNCTION__); }

  template <typename FormatStringProvider, typename Ts, size_t... I>
  const LoggerBase &internalLogf(LogSeverity msg_sev, Ts log_items_tup,
                                 std::index_sequence<I...>) const {

    static constexpr auto Splits =
        cxx17::split(cxx17::appendLf(cxx17::getCStr<FormatStringProvider>()));
    static_assert(sizeof...(I) + 1 == Splits.size(),
                  "Invalid number of format arguments");

#ifndef ITST_DISABLE_LOGGER
    auto actual_sev = global_enforced_log_severity.value_or(severity);
    if (actual_sev > msg_sev) {
      return *this;
    }

    LockedFileWriter writer{file_handle};
    printHeader(msg_sev, writer);

    auto writeNonEmpty = [](std::string_view str, FileWriter writer) {
      if (!str.empty())
        writer(str);
    };

    ((writeNonEmpty(Splits[I], writer),
      printAccordingToType(std::get<I>(log_items_tup), writer)),
     ...);

    writer(Splits.back());

#if defined(ITST_DEBUG_LOGGING) && (_MSC_VER)
    flush();
#endif

#endif
    return *this;
  }

  void printHeader(LogSeverity msg_sev, FileWriter writer) const {
    writer("[");
    printTimestamp(writer);
    writer("][");
    writer(to_string(msg_sev));
    writer("][");
    writer(class_name);
    writer("]: ");
  }

  template <typename Writer>
  static void indent(Writer writer, size_t indent_level) {
    static constexpr char Indents[] =
        "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"; // NOLINT
    writer(std::string_view(Indents, std::min<size_t>(indent_level, 15)));
  }

  template <typename T, typename Writer>
  static void printAccordingToType(const T &item, Writer writer,
                                   size_t indent_level = 0) {
    using ElemTy = std::decay_t<T>;
    using BaseElemTy = std::decay_t<std::remove_pointer_t<ElemTy>>;

    if (indent_level) {
      indent(writer, indent_level);
    }

    if constexpr (std::is_convertible_v<T, std::string_view>) {
      writer(std::string_view(item));
    } else if constexpr (std::is_enum_v<ElemTy> &&
                         has_adl_to_string_v<ElemTy>) {
      /// NOTE: Have the case for enums here already, since enums are
      /// printable as integers and we want to give pretty-printing higher
      /// priority NOTE: Explicitly cast to std::string_view, since we now
      /// allow to_string to return sth different than string - it is just
      /// sufficient to be convertible to StringRef
      writer(std::string_view(adl_to_string(item)));
    } else if constexpr (has_str_v<ElemTy>) {
      writer(item.str());
    } else if constexpr (has_toString_v<ElemTy>) {
      writer(item.toString());
    } else if constexpr (has_adl_to_string_v<ElemTy>) {
      writer(std::string_view(adl_to_string(item)));
    } else if constexpr (is_printable_v<ElemTy>) {
      std::ostringstream os;
      os << item;
      writer(os.str());
    } else if constexpr (is_iterable_v<ElemTy>) {
      writer("{\n");
      for (const auto &sub_element : item) {
        writer("\t");
        printAccordingToType(sub_element, writer, indent_level + 1);
        writer("\n");
      }
      indent(writer, indent_level);
      writer("}\n");
    } else {
      // hint: static_assert is expected to be at compile time not runtime
      static_assert(is_printable_v<ElemTy>,
                    "The type of the logged data is not printable. Please add "
                    "an appropriate overload of "
                    "operator<<(llvm::raw_ostream&, const ElemTy&) or convert "
                    "it to a printable type, e.g. std::string "
                    "before logging");
    }
  }

  template <typename T, typename Writer>
  static void printAccordingToType(const std::optional<T> &item, Writer writer,
                                   size_t indent_level = 0) {
    if (!item) {
      writer("<none>");
    } else {
      printAccordingToType(*item, writer, indent_level);
    }
  }

  static void printTimestamp(FileWriter writer);

  StreamType file_handle{};
  std::string_view class_name{};
  LogSeverity severity{};
};
} // namespace itst
