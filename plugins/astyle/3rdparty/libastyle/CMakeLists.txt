set(astylelib_SRCS
    ASBeautifier.cpp
    ASEnhancer.cpp
    ASFormatter.cpp
    ASResource.cpp
    ASLocalizer.cpp
)
add_library(astylelib STATIC ${astylelib_SRCS})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # astyle lib throws lots of warnings with -Wdocumentation
    add_target_compile_flag_if_supported(astylelib PRIVATE "-Wno-documentation")
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_target_compile_flag_if_supported(astylelib PRIVATE "-Wno-suggest-override")
endif()
# add as SYSTEM include dir so compiler does not emit warnings for the lib headers
target_include_directories(astylelib SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
