# insect-logger
A simple, lightweight logger with minimalistic dependencies

## Getting started

After linking to `libinsect_logger`, just include the `Logger.hpp` header:

```C++
#include "itst/Logger.hpp"
```

## Building

The insect logger can be built using the default cmake process:

```Bash
cd insect-logger
mkdir -p build && cd build
cmake -G Ninja ..
ninja
```

To include the insect logger into your project, you may want to `add_subdirectory()` it from your `CMakeLists.txt`.

## Logging

The insect logger provides several ways of logging that are detailed below.

All symbols exported by the insect logger live in the namespace `itst` or its sub-namespaces.
The examples below assume, you are `using namespace itst;` or working in the namespace `itst`.

The macros are namespace-independent and automatically navigate to `::itst::*`.

### Basic Usage

The following loggers are available currently:

- `ConsoleLogger`: Prints into `stderr`. This logger is `constexpr` initializable.
- `FileLogger`: Prints into the specified file. Appends to the file if it already exists.
- `StringLogger`: Writes the logged content into a string that can be retrieved via `.str()`.

Sample use:

```C++

FileLogger logger(/*filename: */"output.log", /*category: */"main");

logger.logInfo("Dies ist ein Test ", 42, ".");
// equivalent to:
ITST_LOG(Info, "Dies ist ein Test ", 42, ".");
ITST_LOGF(Info, "Dies ist ein Test {}.", 42);
```

The macros require a logger named `logger` to be available in the current scope.

If you don't have a logger available and anyway only want a oneshot `ConsoleLogger`, you can use the `ITST_LOGGER_LOG` and `ITST_LOGGER_LOGF` macros instead.

```C++
ITST_LOGGER_LOG(Info, "Dies ist ein Test ", 42, ".");
ITST_LOGGER_LOGF(Info, "Dies ist ein Test {}.", 42);
```

These macros then create a `static constexpr ConsoleLogger` in a local scope and assign it the name of the enclosing function as log-category.

To manually create such a simple logger instance, there are a few utility macros:

```C++
{
    ITST_LOGGER;
}

{
    ITST_LOGGER_CAT("my_own_fancy_category");
}

{
    ITST_LOGGER_SEV(Warning); // Overriding the default logger severity
}

{
    ITST_LOGGER_CAT_SEV("my_own_fancy_category", Warning); 
    // Equivalent to
    // static constexpr ConsoleLogger("my_own_fancy_category", LogSeverity::Warning);
}
```

### Streaming

Besides the `log*()` functions, the insect logger also supports a streaming interface that allows seamless integration with code that used a different logger previously (e.g., Boost log).

The streaming interface is provided by the `logger.stream(severity)` function and can be used as follows:

```C++
ITST_LOGGER;

ITST_LOG_STREAM(Info) << "Dies ist ein Test " << 42;
// equivalent to:
logger.stream(itst::LogSeverity::Info) << "Dies ist ein Test " << 42;
// equivalent to:
logger.logInfo("Dies ist ein Test ", 42);
```

The `stream()` function returns a temporary `LogStream` that locks the corresponding `logger` and prints the header.
The overloaded `operator<<` uses the same underlying mechanism as the `log*()` functions for formatting the individual parts of the message.
On destruction, the log message is completed with a line-feed and the `logger` gets unlocked.

Note, that the `LogStream` is meant to be used as temporary object for streaming only, so it should not be stored in a variable, or returned from a function.

### Message Format

Each log message is formatted as follows:

`[timestamp][severity][category]: content\n`

The `timestamp` is in the format `YYYY-MM-DD hh:mm:ss.xxxxxx` and the severity is in CAPS (`Fatal` is spelled `CRITICAL`).

### Log Severity

Each logger and log message is assigned one severity level of

- Trace
- Debug
- Info
- Warning
- Error
- Fatal

in increasing severity.

The logger only prints a message if the message's severity is at least the severity of the logger itself.
The default logger-severity is `Info` (if you don't specify one by yourself), but can be changed to `Debug` if compiling with `-DITST_DEBUG_LOGGING`.

If you want to overwrite the severity of all loggers at runtime, you can set the variable `LoggerBase::global_enforced_log_severity` to the desired severity.
Note, that this API is *not* thread-safe.

### Assertions

The assertion system in C/C++ is very primitive not very usable, so the insect logger comes with its own assertion macros.

```C++
ITST_LOGGER;

ITST_ASSERT(x != 0, "x (", x, ") should not be zero");
ITST_ASSERTF(x != 0, "x ({}) should not be zero", x);
```

These assertions write into an already available logger called `logger`.

If you don't have such a logger instance available, you may want to use

```C++
ITST_LOGGER_ASSERT(x != 0, "x (", x, ") should not be zero");
ITST_LOGGER_ASSERTF(x != 0, "x ({}) should not be zero", x);
```
