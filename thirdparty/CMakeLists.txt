set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

add_library(gtest STATIC
    gtest/src/gtest-all.cc
)

target_include_directories(gtest
    PUBLIC gtest/include
    PRIVATE gtest
)

target_link_libraries(gtest pthread)

# Lemon parser generator taken from SQLite source-tree version 3080900.
add_executable(lemon lemon/lemon.c)