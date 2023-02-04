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

class ITST_API LoggerBase {
public:
  static constexpr size_t TabWidth = 4;

  static std::optional<LogSeverity> global_enforced_log_severity;

  static constexpr size_t getTimestepLength() noexcept {
    return sizeof("2022-11-02 15:10:22.633977") - 1;
  }

  static constexpr LogSeverity DefaultSeverity =
#ifdef ITST_DEBUG_LOGGING
      LogSeverity::Debug
#else
      LogSeverity::Info
#endif
      ;

protected:
  explicit constexpr LoggerBase(std::string_view class_name,
                                LogSeverity sev) noexcept
      : class_name(class_name), severity(sev) {}

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

  static void flushImpl(FILE *file_handle) noexcept;

  template <typename Writer>
  static void indent(Writer writer, size_t indent_level) {
    static constexpr char Indents[] =
        "                                "; // NOLINT
    writer(std::string_view(Indents, std::min<size_t>(indent_level * TabWidth,
                                                      sizeof(Indents) - 1)));
  }

  static void printTimestamp(FileWriter writer);

  void printHeader(LogSeverity msg_sev, FileWriter writer) const {
    writer("[");
    printTimestamp(writer);
    writer("][");
    writer(to_string(msg_sev));
    writer("][");
    writer(class_name);
    writer("]: ");
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

  template <typename... Ts>
  void logImpl(FILE *file_handle, LogSeverity msg_sev,
               const Ts &...log_items) const {
#ifndef ITST_DISABLE_LOGGER
    auto actual_sev = global_enforced_log_severity.value_or(severity);
    if (actual_sev > msg_sev) {
      return;
    }

    LockedFileWriter writer{file_handle};

    printHeader(msg_sev, writer);

    (printAccordingToType(log_items, writer), ...);

    writer("\n");

#if defined(ITST_DEBUG_LOGGING) && (_MSC_VER)
    flush();
#endif

#endif
  }

  template <typename FormatStringProvider, typename Ts, size_t... I>
  void internalLogf(FILE *file_handle, LogSeverity msg_sev, Ts log_items_tup,
                    std::index_sequence<I...>) const {

    static constexpr auto Splits = cxx17::splitFormatString(
        cxx17::appendLf(cxx17::getCStr<FormatStringProvider>()));
    static_assert(sizeof...(I) + 1 == std::tuple_size_v<decltype(Splits)>,
                  "Invalid number of format arguments");

#ifndef ITST_DISABLE_LOGGER
    auto actual_sev = global_enforced_log_severity.value_or(severity);
    if (actual_sev > msg_sev) {
      return;
    }

    LockedFileWriter writer{file_handle};
    printHeader(msg_sev, writer);

    auto writeNonEmpty = [](std::string_view str, FileWriter writer) {
      if (!str.empty())
        writer(str);
    };

    ((writeNonEmpty(std::get<I>(Splits).str(), writer),
      printAccordingToType(std::get<I>(log_items_tup), writer)),
     ...);

    writer(std::get<sizeof...(I)>(Splits).str());

#if defined(ITST_DEBUG_LOGGING) && (_MSC_VER)
    flush();
#endif

#endif
  }

  // ---

  std::string_view class_name{};
  LogSeverity severity{};
};

/// An efficient, lightweight and thread-safe logger.
/// The only thing not thread-safe is global_enforced_log_severity; however,
/// it is expected to be set once at the beginning and never changed again.
template <typename Derived> class LoggerImpl : public LoggerBase {
public:
  explicit constexpr LoggerImpl(std::string_view class_name,
                                LogSeverity sev) noexcept
      : LoggerBase(class_name, sev) {}

  template <typename... Ts>
  const LoggerImpl &log(LogSeverity msg_sev, const Ts &...log_items) const {
    logImpl(self().getFileHandle(), msg_sev, log_items...);
    return *this;
  }

  template <typename FormatStringProvider, typename... Ts>
  const LoggerImpl &logf(FormatStringProvider /*FSP*/, LogSeverity msg_sev,
                         const Ts &...log_items) const {
    internalLogf<FormatStringProvider>(
        self().getFileHandle(), msg_sev, std::tie(log_items...),
        std::make_index_sequence<sizeof...(Ts)>());
    return *this;
  }

  template <typename... Ts>
  const LoggerImpl &logTrace(const Ts &...log_items) const {
    return log(LogSeverity::Trace, log_items...);
  }
  template <typename... Ts>
  const LoggerImpl &logDebug(const Ts &...log_items) const {
    return log(LogSeverity::Debug, log_items...);
  }
  template <typename... Ts>
  const LoggerImpl &logInfo(const Ts &...log_items) const {
    return log(LogSeverity::Info, log_items...);
  }
  template <typename... Ts>
  const LoggerImpl &logWarning(const Ts &...log_items) const {
    return log(LogSeverity::Warning, log_items...);
  }
  template <typename... Ts>
  const LoggerImpl &logError(const Ts &...log_items) const {
    return log(LogSeverity::Error, log_items...);
  }
  template <typename... Ts>
  const LoggerImpl &logFatal(const Ts &...log_items) const {
    return log(LogSeverity::Fatal, log_items...);
  }

  void flush() const { flushImpl(self().getFileHandle()); }

  // template <typename T> static std::string log_string(const T &item) { //
  // NOLINT
  //  std::string ret;
  //  llvm::raw_string_ostream ros(ret);
  //  printAccordingToType(item, [&ros](std::string_view str) { ros << str;
  //  }); return ret;
  //}

private:
  constexpr const Derived &self() const noexcept {
    return static_cast<const Derived &>(*this);
  }
};
} // namespace itst
