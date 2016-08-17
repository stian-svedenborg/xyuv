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

# Make headers utility downloaded from http://www.hwaci.com/sw/mkhdr/makeheaders.c on Aug 17. 2016.
set_source_files_properties(makeheaders/makeheaders.c PROPERTIES COMPILE_FLAGS "-Wno-error -Wno-unused-but-set-variable")
add_executable(makeheaders makeheaders/makeheaders.c)