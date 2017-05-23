
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
#if(COMMAND cmake_policy)
#      cmake_policy(SET CMP0003 NEW)
#endif(COMMAND cmake_policy)

set(MINICALC_TESTING_SOURCES
        ../xyuv/src/config-parser/minicalc/minicalc.cpp
        ../xyuv/src/config-parser/minicalc/parser.cpp
        ../xyuv/src/config-parser/minicalc/ast.cpp
        ../xyuv/src/config-parser/minicalc/operations.cpp
        )


set(INTEGRATION_TESTING_SOURCES
    integration_testing/main.cpp
    integration_testing/regress_format_templates.cpp
    integration_testing/file_format_testing.cpp

    TestResources.cpp
    TestResources.h
    integration_testing/conversion_matrices.cpp
)

if (ImageMagick_FOUND)
    list(APPEND INTEGRATION_TESTING_SOURCES
            integration_testing/external/imagick_test.cpp)
endif(ImageMagick_FOUND)

if (PNG_FOUND)
    list(APPEND INTEGRATION_TESTING_SOURCES
            integration_testing/external/libpng_test.cpp)
endif(PNG_FOUND)

# Add targets
add_executable(minicalc
        ${MINICALC_TESTING_SOURCES}
)

add_executable(unit_tests
        main.cpp
        parsing_test.cpp
        utility_testing.cpp
        yuv_image_testing.cpp
        rgb_test.cpp
        unorm_test.cpp
        sampling_test.cpp
        TestResources.cpp
        TestResources.h
        continuation_blocks.cpp
        interleave_test.cpp
        bit_packing.cpp block_reorder.cpp)

add_executable(integration_testing
        ${INTEGRATION_TESTING_SOURCES}
)

target_include_directories(unit_tests
    PRIVATE xyuv.PUBLIC
    PRIVATE gtest.PUBLIC
)

target_include_directories(integration_testing
    PRIVATE xyuv.PUBLIC
    PRIVATE gtest.PUBLIC
)

if(PNG_FOUND)
    target_include_directories(integration_testing
            PUBLIC ${PNG_INCLUDE_DIRS})
    target_link_libraries(integration_testing
            PUBLIC ${PNG_LIBRARIES})
    target_compile_definitions(integration_testing
            PUBLIC ${PNG_DEFINITIONS})
endif(PNG_FOUND)

if (ImageMagick_FOUND)
    target_include_directories(integration_testing
            PUBLIC ${ImageMagick_INCLUDE_DIRS})
    target_link_libraries(integration_testing
            PUBLIC ${ImageMagick_LIBRARIES})
endif(ImageMagick_FOUND)

target_link_libraries(unit_tests
        PUBLIC xyuv
        PUBLIC gtest)
target_link_libraries(integration_testing
        PUBLIC xyuv
        PUBLIC gtest )

target_compile_definitions(minicalc PRIVATE "-DCOMPILE_TEST_PROGRAM")
add_dependencies(unit_tests gtest xyuv)
add_dependencies(integration_testing gtest xyuv)