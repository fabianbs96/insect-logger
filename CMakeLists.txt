cmake_minimum_required (VERSION 3.10)

# Allow portable use of CMAKE_VISIBILITY_INLINES_HIDDEN not only for shared libraries
cmake_policy(SET CMP0063 NEW)
cmake_policy(SET CMP0077 NEW)
cmake_policy(SET CMP0126 NEW)

project (InsectLogger
    LANGUAGES CXX
)
set(CMAKE_PROJECT_NAME "InsectLogger")

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ITST_BUILD_SHARED_LIB "Build a shared library (.so on UNIX-like systems, .dll on Windows) Default is OFF" OFF)
option(ITST_DEBUG_LOGGING "Set the default log-severity to DEBUG (otherwise, the default is INFO)" OFF)
option(ITST_DISABLE_LOGGER "Disable all logging statically. It cannot be turned on at runtime (default is OFF). You may want to enable this in high-performance scenarios" OFF)
option(ITST_DISABLE_ASSERT "Disable the custom ITST_ASSERT macro. Useful in release builds for optimization (default is OFF)" OFF)
option(ITST_ENABLE_COLORS "Enable colored log messages for the console logger (default is ON)" ON)

set(ITST_CONSOLE_LOGGER_TARGET_DEFAULT "stderr")
if (DEFINED ITST_CONSOLE_LOGGER_TARGET)
  set(ITST_CONSOLE_LOGGER_TARGET_DEFAULT "${ITST_CONSOLE_LOGGER_TARGET}")
endif()
set(ITST_CONSOLE_LOGGER_TARGET "${ITST_CONSOLE_LOGGER_TARGET_DEFAULT}" CACHE STRING "The target for the ConsoleLogger. Either 'stdout' or 'stderr', defaults to 'stderr'")
set_property(CACHE ITST_CONSOLE_LOGGER_TARGET PROPERTY STRINGS "stderr" "stdout")

option(CMAKE_VISIBILITY_INLINES_HIDDEN "Hide inlined functions from the DSO table (default ON)" ON)

string(APPEND CMAKE_CXX_FLAGS " -MP -fstack-protector-strong -ffunction-sections -fdata-sections -pipe")
string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " -fno-omit-frame-pointer")
string(APPEND CMAKE_CXX_FLAGS_RELEASE "")

add_library(insect_logger_includes INTERFACE EXCLUDE_FROM_ALL)
target_include_directories(insect_logger_includes INTERFACE include/)
target_compile_features(insect_logger_includes INTERFACE cxx_std_17)

add_subdirectory(src)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sample")
    message(STATUS "Found sample directory")
    add_subdirectory(sample/)
endif()
