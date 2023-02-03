cmake_minimum_required (VERSION 3.10)

project (InsectLogger)
set(CMAKE_PROJECT_NAME "InsectLogger")


set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ITST_BUILD_SHARED_LIB "Build a shared library (.so on UNIX-like systems, .dll on Windows) Default is OFF" OFF)
option(ITST_DEBUG_LOGGING "Set the default log-severity to DEBUG (otherwise, the default is INFO)" OFF)
if(ITST_DEBUG_LOGGING)
    add_compile_definitions(ITST_DEBUG_LOGGING)
endif()
option(ITST_DISABLE_LOGGER "Disable all logging statically. It cannot be turned on at runtime (default is OFF). You may want to enable this in high-performance scenarios" OFF)
if(ITST_DISABLE_LOGGER)
    add_compile_definitions(ITST_DISABLE_LOGGER)
endif()
option(ITST_DISABLE_ASSERT "Disable the custom ITST_ASSERT macro. Useful in release builds for optimization (default OFF)" OFF)
if(ITST_DISABLE_ASSERT)
    add_compile_definitions(ITST_DISABLE_ASSERT)
endif()

include_directories(include/)
add_subdirectory(src)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sample")
    message(STATUS "Found sample directory")
    add_subdirectory(sample/)
endif()