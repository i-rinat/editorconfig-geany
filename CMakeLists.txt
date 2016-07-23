project(editorconfig-geany-plugin)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fPIC -pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror=format-security")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=implicit-function-declaration")

find_package(PkgConfig REQUIRED)
pkg_check_modules(REQ geany REQUIRED)

include_directories(${REQ_INCLUDE_DIRS})

add_library(editorconfig-geany SHARED
    editorconfig-geany.c
)

target_link_libraries(editorconfig-geany
    ${REQ_LIBRARIES}
    -leditorconfig  # TODO: search for the library rather than hardcode
)

string(REPLACE ";" " " REQ_CFLAGS_OTHER_STR "${REQ_CFLAGS_OTHER}")

set_target_properties(editorconfig-geany
    PROPERTIES
    COMPILE_FLAGS "${REQ_CFLAGS_OTHER_STR}"
    PREFIX ""
)
