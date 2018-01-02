project(rapidcsv LANGUAGES CXX)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_VERBOSE_MAKEFILE ON)

if (MSVC)
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]") # https://msdn.microsoft.com/en-us/library/fwkeyyhe.aspx
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif ()
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wpedantic -Wshadow -Wpointer-arith \
                       -Wcast-qual -Wno-missing-braces -Wswitch-default -Wcast-align -Wunreachable-code \
                       -Wundef -Wuninitialized")
endif ()

add_compile_options("$<$<STREQUAL:$<UPPER_CASE:CMAKE_BUILD_TYPE>,DEBUG>:
    $<$<BOOL:MSVC>: /Zi>
    $<$<NOT:$<BOOL:MSVC>>: -g >>")

# rapidcsv library
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if (CMAKE_SYSTEM_NAME STREQUAL Windows)
    target_compile_definitions(${PROJECT_NAME} INTERFACE DEFAULT_HASCR=1)
endif()


# Examples
add_subdirectory(examples)

# Tests
enable_testing()
add_subdirectory(tests)
