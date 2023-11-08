file(GLOB_RECURSE ITST_LOGGER_SRC *.cpp)

if(${ITST_BUILD_SHARED_LIB})
    add_library(insect_logger SHARED
        ${ITST_LOGGER_SRC}
    )

    target_compile_definitions(insect_logger PUBLIC ITST_SHARED_LIB)
    if(WIN32)
        target_compile_definitions(insect_logger PRIVATE ITST_BUILD_DLL)
    endif()

else()
    add_library(insect_logger STATIC
        ${ITST_LOGGER_SRC}
    )
endif()

target_link_libraries(insect_logger PUBLIC insect_logger_includes)
if(ITST_DEBUG_LOGGING)
    target_compile_definitions(insect_logger PUBLIC ITST_DEBUG_LOGGING)
endif()
if(ITST_DISABLE_LOGGER)
    target_compile_definitions(insect_logger PUBLIC ITST_DISABLE_LOGGER)
endif()
if(ITST_DISABLE_ASSERT)
    target_compile_definitions(insect_logger PUBLIC ITST_DISABLE_ASSERT)
endif()
