cmake_minimum_required(VERSION 3.1)
project(crossyuv)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Make errors visible.
add_compile_options(
    "-Wall"
    "-mcmodel=large"
    "-m64"
    "-Werror"
)

# Configurable Options
option(BUILD_SHARED_LIBS
    "Build top level libraries as shared." ON)

option(USE_IMAGEMAGICK_INTEGRATION
    "Include module to interact with ImageMagick++" ON)

# Register further options.

set(MAGICKCORE_QUANTUM_DEPTH 16 CACHE STRING
    "If USE_IMAGEMAGICK_INTEGRATION is on, then this constant is passed on to the imagemagick headers. See ImageMagick Docs for more info." FORCE)


set(MAGICKCORE_HDRI_ENABLE 0 CACHE STRING
    "If USE_IMAGEMAGICK_INTEGRATION is on, then this constant is passed on to the imagemagick headers. See ImageMagick Docs for more info." FORCE)

# Primary library.
add_library(xyuv 
    xyuv/src/xyuv.cpp
)

target_include_directories(xyuv
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xyuv/include>
    PUBLIC $<INSTALL_INTERFACE:xyuv/include>
)

# Primary library
add_library(xyuv_core STATIC
    xyuv/src/pixel_packer.cpp
    xyuv/src/rgb_conversion.cpp
    xyuv/src/utility.cpp
    xyuv/src/config-parser/chroma_siting_parser.cpp
    xyuv/src/config-parser/pixel_format_parser.cpp
    xyuv/src/config-parser/read_json.cpp
    xyuv/src/config-parser/rgb_conversion_parser.cpp
    xyuv/src/config-parser/minicalc/ast.cpp
    xyuv/src/config-parser/minicalc/minicalc.cpp
    xyuv/src/config-parser/minicalc/parser.cpp
    xyuv/src/config-parser/minicalc/operations.cpp
    xyuv/src/config-parser/minicalc_entry_point.cpp
    xyuv/src/io/xyuv_io.cpp
    xyuv/src/io/io_structs.cpp
    xyuv/src/format.cpp
    xyuv/src/yuv_image.cpp
    xyuv/src/subsampler.cpp
    xyuv/src/coder.cpp
    xyuv/src/rgb_image.cpp
    xyuv/src/config-parser/config_manager.cpp
    xyuv/src/frame_manipulation.cpp
    xyuv/src/comparison_operators.cpp
    xyuv/src/config-parser/parse_error.h
    xyuv/src/config-parser/parse_helpers.cpp
    xyuv/include/xyuv/large_buffer.h
    xyuv/src/config-parser/format_validator.h
    xyuv/src/config-parser/format_validator.cpp
)

target_include_directories(xyuv_core
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xyuv/include>
    PUBLIC $<INSTALL_INTERFACE:xyuv/include>
    PRIVATE thirdparty/rapidjson/include
)

add_dependencies(xyuv xyuv_core)
target_link_libraries(xyuv xyuv_core)

if (USE_IMAGEMAGICK_INTEGRATION)

    # Resolve dependencies
    find_package(ImageMagick REQUIRED COMPONENTS Magick++)
    message("Found ImageMagick headers at ${ImageMagick_INCLUDE_DIRS}")

    # Set up library
    add_library(xyuv_magick STATIC
        xyuv/src/imagemagick/magick_wrapper.cpp
    )

    target_include_directories(xyuv_magick
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xyuv/include/xyuv/imagemagick>
        PUBLIC $<INSTALL_INTERFACE:xyuv/include/xyuv/imagemagick>
        PRIVATE xyuv/include
        PUBLIC ${ImageMagick_INCLUDE_DIRS}
    )

    add_dependencies(xyuv_magick xyuv_core)
    target_link_libraries(xyuv_magick ${ImageMagick_LIBRARIES} xyuv_core)

    # Register dependencies to shared library.
    add_dependencies(xyuv xyuv_magick)
    target_link_libraries(xyuv xyuv_magick)

    target_compile_options(xyuv_magick
        PUBLIC -DMAGICKCORE_QUANTUM_DEPTH=${MAGICKCORE_QUANTUM_DEPTH}
        PUBLIC -DMAGICKCORE_HDRI_ENABLE=${MAGICKCORE_HDRI_ENABLE}
    )

endif (USE_IMAGEMAGICK_INTEGRATION)

#Add subdirectories for other targets.
add_subdirectory(thirdparty)
add_subdirectory(testing)
add_subdirectory(utilities)

# Installation section.
install (
    TARGETS xyuv_core xyuv_magick
    DESTINATION lib/static
    EXPORT lib
)
install (
    TARGETS xyuv
    DESTINATION lib
    EXPORT lib
)
install (
    DIRECTORY xyuv/include/
    DESTINATION include
    FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
)
install (
    DIRECTORY formats/
    DESTINATION share/xyuv
    FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
)
install (
    EXPORT lib
    NAMESPACE xyuv_
    DESTINATION lib/xyuv
)



