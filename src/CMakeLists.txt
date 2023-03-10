file(GLOB_RECURSE ITST_LOGGER_SRC *.cpp)

if(${ITST_BUILD_SHARED_LIB})
    add_compile_definitions(ITST_SHARED_LIB)
    if(WIN32)
        add_compile_definitions(ITST_BUILD_DLL)
    endif()

    add_library(insect_logger SHARED
        ${ITST_LOGGER_SRC}
    )

else()
    add_library(insect_logger STATIC
        ${ITST_LOGGER_SRC}
    )
endif()
