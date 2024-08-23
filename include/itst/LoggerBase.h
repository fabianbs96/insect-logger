#pragma once

#include "itst/Core.h"
#include "itst/LogSeverity.h"
#include "itst/common/TemplateString.h"
#include "itst/common/TypeTraits.h"

#include <cassert>
#include <charconv>
#include <cstdio>
#include <limits>
#include <optional>
#include <sstream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace itst {

class ITST_API LoggerBase {
public:
  template <typename U> friend class LogStream;

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

  struct ITST_API [[clang::trivial_abi]] FileLock {
#if defined(_GNU_SOURCE) && !defined(ITST_DISABLE_LOGGER)
    static FileLock create(FILE *file_handle) noexcept;

    FileLock(FileLock &&other) noexcept
        : file_handle{std::exchange(other.file_handle, nullptr)} {}
    FileLock &operator=(FileLock &&other) noexcept {
      std::swap(file_handle, other.file_handle);
      return *this;
    }
    void destroy() noexcept;
    ~FileLock() {
      if (file_handle)
        destroy();
    }
#else
    static FileLock create(FILE *file_handle) noexcept { return {}; }
#endif

    explicit operator bool() const noexcept { return file_handle != nullptr; }

    FILE *file_handle{};

  private:
    FileLock() noexcept {}
  };

  static void flushImpl(FILE *file_handle) noexcept;

  template <typename Writer>
  static void indent(Writer writer, size_t indent_level) noexcept {
    static constexpr char Indents[] =
        "                                "; // NOLINT
    writer(std::string_view(Indents, std::min<size_t>(indent_level * TabWidth,
                                                      sizeof(Indents) - 1)));
  }

  static void printTimestamp(FileWriter writer) noexcept;

  void printHeader(LogSeverity msg_sev, FileWriter writer) const noexcept;

  template <typename T, typename Writer>
  static constexpr bool isPrintNoexcept() {
    if (!noexcept(std::declval<Writer>()("")))
      return false;

    using ElemTy = std::decay_t<T>;

    if constexpr (has_log_traits_v<T, Writer>) {
      return noexcept(LogTraits<T>::printAccordingToType(
          std::declval<const T &>(), std::declval<Writer>()));
    } else if constexpr (std::is_convertible_v<T, std::string_view>) {
      return std::is_convertible_v<T, std::string_view>;
    } else if constexpr (std::is_enum_v<ElemTy> &&
                         has_adl_to_string_v<ElemTy>) {
      return noexcept(adl_to_string(std::declval<ElemTy>()));
    } else if constexpr (std::is_integral_v<ElemTy>) {
      return true;
    } else if constexpr (has_str_v<ElemTy>) {
      return noexcept(std::declval<const T &>().str());
    } else if constexpr (has_toString_v<ElemTy>) {
      return noexcept(std::declval<const T &>().toString());
    } else if constexpr (has_adl_to_string_v<ElemTy>) {
      return is_nothrow_to_string<ElemTy>();
    } else if constexpr (is_printable_v<ElemTy>) {
      return false;
    } else if constexpr (is_iterable_v<ElemTy>) {
      using std::begin;
      return isPrintNoexcept<
          std::decay_t<decltype(*begin(std::declval<ElemTy>()))>, Writer>();

    } else {
      return false;
    }
  }

  template <typename T, typename Writer>
  static void printAccordingToType(
      const T &item, Writer writer,
      size_t indent_level = 0) noexcept(isPrintNoexcept<T, Writer>()) {
    using ElemTy = std::decay_t<T>;

    if (indent_level) {
      indent(writer, indent_level);
    }

    if constexpr (has_log_traits_v<T, Writer>) {
      LogTraits<T>::printAccordingToType(item, writer);
    } else if constexpr (std::is_convertible_v<T, std::string_view>) {
      writer(std::string_view(item));
    } else if constexpr (std::is_enum_v<ElemTy> &&
                         has_adl_to_string_v<ElemTy>) {
      /// NOTE: Have the case for enums here already, since enums are
      /// printable as integers and we want to give pretty-printing higher
      /// priority NOTE: Explicitly cast to std::string_view, since we now
      /// allow to_string to return sth different than string - it is just
      /// sufficient to be convertible to string_view
      writer(std::string_view(adl_to_string(item)));
    } else if constexpr (std::is_same_v<ElemTy, bool>) {
      writer(item ? "true" : "false");
    } else if constexpr (std::is_integral_v<ElemTy>) {
      std::array<char, sizeof("18446744073709551615")> buf{};
      auto [ptr, err] =
          std::to_chars(buf.data(), buf.data() + buf.size(), item, 10);
      writer(std::string_view(buf.data(), ptr - buf.data()));
    } else if constexpr (std::is_floating_point_v<ElemTy>) {
      // For the buffer-size, see the libstdc++ impl of std::to_string
      std::array<char, std::numeric_limits<ElemTy>::max_exponent10 + 20> buf{};
      ptrdiff_t Len{};
      constexpr const char *Fmt =
          std::is_same_v<long double, ElemTy> ? "%Lg" : "%g";
      Len = snprintf(buf.data(), buf.size(), Fmt, item);
      writer(std::string_view(buf.data(), Len));
    } else if constexpr (has_str_v<ElemTy>) {
      writer(item.str());
    } else if constexpr (has_toString_v<ElemTy>) {
      writer(item.toString());
    } else if constexpr (has_adl_to_string_v<ElemTy>) {
      writer(std::string_view(adl_to_string(item)));
    } else if constexpr (is_printable_v<ElemTy>) {
      std::ostringstream os;
      os << item;
#if __cplusplus >= 202002L
      writer(os.view());
#else
      writer(os.str());
#endif
    } else if constexpr (is_iterable_v<ElemTy>) {
      writer("{\n");
      for (const auto &sub_element : item) {
        printAccordingToType(sub_element, writer, indent_level + 1);
        writer("\n");
      }
      indent(writer, indent_level);
      writer("}");
    } else {
      // hint: static_assert is expected to be at compile time not runtime
      static_assert(is_printable_v<ElemTy>,
                    "The type of the logged data is not printable. Please add "
                    "an appropriate overload of "
                    "operator<<(std::ostream&, const ElemTy&) or "
                    "to_string(const ElemTy&) or convert "
                    "it to a printable type, e.g. std::string "
                    "before logging");
    }
  }

  template <typename T, typename Writer>
  static void printAccordingToType(const std::optional<T> &item, Writer writer,
                                   size_t indent_level = 0) {
    if (indent_level) {
      indent(writer, indent_level);
    }
    if (!item) {
      writer("<none>");
    } else {
      writer("<some ");
      printAccordingToType(*item, writer, 0);
      writer(">");
    }
  }

  FileLock startLogging(FILE *file_handle, LogSeverity msg_sev) const noexcept {
#ifndef ITST_DISABLE_LOGGER

    auto actual_sev = global_enforced_log_severity.value_or(severity);
    bool filter_logging = actual_sev > msg_sev;
    auto lock = FileLock::create(filter_logging ? nullptr : file_handle);
    if (filter_logging) {
      return lock;
    }

    printHeader(msg_sev, FileWriter{file_handle});

    return lock;
#else
    return FileLock::create(nullptr);
#endif // ITST_DISABLE_LOGGER
  }

  void endLoggingWithLF(FileLock lock) const noexcept {
#ifndef ITST_DISABLE_LOGGER
    if (lock) {
      FileWriter{lock.file_handle}("\n");
#if defined(ITST_DEBUG_LOGGING) && (_MSC_VER)
      flush();
#endif
    }
#endif // ITST_DISABLE_LOGGER
  }

  void endLogging([[maybe_unused]] FileLock lock) const noexcept {
#ifndef ITST_DISABLE_LOGGER
#if defined(ITST_DEBUG_LOGGING) && (_MSC_VER)
    if (lock) {
      flush();
    }
#endif
#endif // ITST_DISABLE_LOGGER
  }

  template <typename... Ts>
  void logImpl(FILE *file_handle, LogSeverity msg_sev,
               const Ts &...log_items) const
      noexcept((... && isPrintNoexcept<Ts, FileWriter>())) {
#ifndef ITST_DISABLE_LOGGER
    if (auto lock = startLogging(file_handle, msg_sev)) {
      (printAccordingToType(log_items, FileWriter{file_handle}), ...);
      FileWriter{file_handle}("\n");
      endLogging(std::move(lock));
    }
#endif
  }

  template <typename FormatStringProvider, typename Ts, size_t... I>
  void internalLogf(FILE *file_handle, LogSeverity msg_sev, Ts log_items_tup,
                    std::index_sequence<I...>) const {

    static constexpr auto Splits = cxx17::splitFormatString(
        cxx17::appendLf(cxx17::getCStr<FormatStringProvider>()));
    static_assert(sizeof...(I) + 1 <= std::tuple_size_v<decltype(Splits)>,
                  "Not enough format arguments specified");
    static_assert(sizeof...(I) + 1 >= std::tuple_size_v<decltype(Splits)>,
                  "Too many format arguments specified");

#ifndef ITST_DISABLE_LOGGER
    // Note: Wrap the following into an if constexpr, to prevent subsequent
    // errors after the static_assert
    if constexpr (sizeof...(I) + 1 == std::tuple_size_v<decltype(Splits)>) {
      if (auto lock = startLogging(file_handle, msg_sev)) {
        FileWriter writer{file_handle};
        constexpr auto writeNonEmpty = [](auto str, FileWriter writer) {
          if constexpr (!str.str().empty())
            writer(str.str());
        };

        ((writeNonEmpty(std::get<I>(Splits), writer),
          printAccordingToType(std::get<I>(log_items_tup), writer)),
         ...);

        writeNonEmpty(std::get<sizeof...(I)>(Splits), writer);
        endLogging(std::move(lock));
      }
    }
#endif
  }

  // ---

  std::string_view class_name{};
  LogSeverity severity{};
};

template <typename U> class LoggerImpl;

template <typename LoggerT> class LogStream {
  template <typename U> friend class LoggerImpl;

public:
  ~LogStream() noexcept;

  template <typename T> const LogStream &operator<<(const T &value) const;

private:
  LogStream(const LoggerImpl<LoggerT> &logger, LogSeverity sev) noexcept;

  // ---
  const LoggerImpl<LoggerT> &logger;
  LoggerBase::FileLock writer;
};

/// An efficient, lightweight and thread-safe logger.
/// The only thing not thread-safe is global_enforced_log_severity; however,
/// it is expected to be set once at the beginning and never changed again.
template <typename Derived> class LoggerImpl : public LoggerBase {
  template <typename U> friend class LogStream;

public:
  explicit constexpr LoggerImpl(std::string_view class_name,
                                LogSeverity sev) noexcept
      : LoggerBase(class_name, sev) {}

  template <typename... Ts>
  const LoggerImpl &log(LogSeverity msg_sev, const Ts &...log_items) const {
#ifndef ITST_DISABLE_LOGGER
    logImpl(self().getFileHandle(), msg_sev, log_items...);
#endif
    return *this;
  }

  template <typename FormatStringProvider, typename... Ts>
  const LoggerImpl &logf(FormatStringProvider /*FSP*/, LogSeverity msg_sev,
                         const Ts &...log_items) const {
#ifndef ITST_DISABLE_LOGGER
    internalLogf<FormatStringProvider>(
        self().getFileHandle(), msg_sev, std::tie(log_items...),
        std::make_index_sequence<sizeof...(Ts)>());
#endif
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

  void flush() const noexcept {
#ifndef ITST_DISABLE_LOGGER
    flushImpl(self().getFileHandle());
#endif
  }

  [[nodiscard]] LogStream<Derived> stream(LogSeverity sev) const noexcept {
    return {*this, sev};
  }

  // template <typename T> static std::string log_string(const T &item) { //
  // NOLINT
  //  std::string ret;
  //  llvm::raw_string_ostream ros(ret);
  //  printAccordingToType(item, [&ros](std::string_view str) { ros << str;
  //  }); return ret;
  //}

private:
  [[nodiscard]] FileLock startLogging(LogSeverity msg_sev) const noexcept {
    return this->LoggerBase::startLogging(self().getFileHandle(), msg_sev);
  }

  [[nodiscard]] constexpr const Derived &self() const noexcept {
    return static_cast<const Derived &>(*this);
  }
};

template <typename LoggerT> inline LogStream<LoggerT>::~LogStream() noexcept {
  logger.endLoggingWithLF(std::move(writer));
}

template <typename LoggerT>
template <typename T>
inline const itst::LogStream<LoggerT> &
LogStream<LoggerT>::operator<<(const T &value) const {
#ifndef ITST_DISABLE_LOGGER
  if (writer)
    logger.printAccordingToType(value,
                                LoggerBase::FileWriter{writer.file_handle});
#endif
  return *this;
}

template <typename LoggerT>
inline LogStream<LoggerT>::LogStream(const LoggerImpl<LoggerT> &logger,
                                     LogSeverity sev) noexcept
    : logger(logger), writer(logger.startLogging(sev)) {}

} // namespace itst
