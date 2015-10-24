
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
#if(COMMAND cmake_policy)
#      cmake_policy(SET CMP0003 NEW)
#endif(COMMAND cmake_policy)

# Add targets
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
        bit_packing.cpp)



add_executable(integration_testing
    integration_testing/main.cpp
    integration_testing/regress_format_templates.cpp
    integration_testing/file_format_testing.cpp

TestResources.cpp TestResources.h)

target_include_directories(unit_tests
    PRIVATE xyuv.PUBLIC
    PRIVATE gtest.PUBLIC
)

target_include_directories(integration_testing
    PRIVATE xyuv.PUBLIC
    PRIVATE gtest.PUBLIC
)

if (USE_IMAGEMAGICK_INTEGRATION)
    target_include_directories(integration_testing
         PRIVATE xyuv_magick.PUBLIC
    )

    target_sources(integration_testing
        PRIVATE integration_testing/imagick_test.cpp
    )

    add_dependencies(integration_testing gtest)
    target_link_libraries(integration_testing ${ImageMagick_Magic++_LIBRARIES})

    target_compile_options(integration_testing
        PRIVATE -DMAGICKCORE_QUANTUM_DEPTH=${MAGICKCORE_QUANTUM_DEPTH}
        PRIVATE -DMAGICKCORE_HDRI_ENABLE=${MAGICKCORE_HDRI_ENABLE}
    )

endif (USE_IMAGEMAGICK_INTEGRATION)

target_link_libraries(unit_tests xyuv gtest)
target_link_libraries(integration_testing xyuv gtest)

add_dependencies(unit_tests gtest xyuv)
add_dependencies(integration_testing gtest xyuv)