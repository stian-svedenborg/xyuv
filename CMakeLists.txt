cmake_minimum_required(VERSION 2.8)
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

option(NO_EXTERNAL_IMAGEMAGICK
    "Do not use imagemagick for xyuv_header and do not include the imagemagick wrapper class." OFF)

option(NO_EXTERNAL_LIBPNG
    "Do not use libpng in xyuv_header and do not include the imagemagick wrapper class." OFF)

# Register further options.

#set(MAGICKCORE_QUANTUM_DEPTH 16 CACHE STRING
#    "If USE_IMAGEMAGICK_INTEGRATION is on, then this constant is passed on to the imagemagick headers. See ImageMagick Docs for more info." FORCE)


#set(MAGICKCORE_HDRI_ENABLE 0 CACHE STRING
#    "If USE_IMAGEMAGICK_INTEGRATION is on, then this constant is passed on to the imagemagick headers. See ImageMagick Docs for more info." FORCE)

# Set up Include primary XYUV sources
set(XYUV_SOURCES
        xyuv/src/pixel_packer.cpp
        xyuv/src/color_conversion.cpp
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
        xyuv/src/format.cpp
        xyuv/src/yuv_image.cpp
        xyuv/src/subsampler.cpp
        xyuv/src/rgb_image.cpp
        xyuv/src/config-parser/config_manager.cpp
        xyuv/src/frame_manipulation.cpp
        xyuv/src/comparison_operators.cpp
        xyuv/src/config-parser/parse_error.h
        xyuv/src/config-parser/parse_helpers.cpp
        xyuv/include/xyuv/large_buffer.h
        xyuv/src/config-parser/format_validator.h
        xyuv/src/config-parser/format_validator.cpp
        xyuv/src/block_reorder.cpp
        xyuv/src/block_reorder.h
        xyuv/src/io/xyuv_io.cpp
        xyuv/src/io/versions/core_io_structs.h
        xyuv/src/io/versions/core_io_structs.cpp
        xyuv/src/io/versions/file_format_entry_point.h

        # File format versions
        xyuv/src/io/versions/0/entry_point.h
        xyuv/src/io/versions/0/entry_point.cpp
        xyuv/src/io/versions/0/file_header.h
        xyuv/src/io/versions/0/io_structs.h
        xyuv/src/io/versions/0/io_structs.cpp

        xyuv/src/io/versions/1/entry_point.h
        xyuv/src/io/versions/1/entry_point.cpp
        xyuv/src/io/versions/1/file_header.h
        xyuv/src/io/versions/1/io_structs.h
        xyuv/src/io/versions/1/io_structs.cpp
        )

# Conditionally include wrappers for external libraries
if(NOT NO_EXTERNAL_LIBPNG)
    find_package(PNG 1.2)
    if (PNG_FOUND)
        list(APPEND XYUV_SOURCES
            xyuv/include/xyuv/external/libpng_wrapper.h
            xyuv/src/external/libpng_wrapper.cpp)
    endif(PNG_FOUND)
endif(NOT NO_EXTERNAL_LIBPNG)

if(NOT NO_EXTERNAL_IMAGEMAGICK)
    find_package(ImageMagick 6.7 COMPONENTS Magick++ MagickCore)
    if(ImageMagick_FOUND)
    list(APPEND XYUV_SOURCES
            xyuv/include/xyuv/external/magick_wrapper.h
            xyuv/src/external/magick_wrapper.cpp)
    endif(ImageMagick_FOUND)
endif(NOT NO_EXTERNAL_IMAGEMAGICK)

add_library(xyuv ${XYUV_SOURCES} )

######################
# Flags and defines  #
######################

SET_TARGET_PROPERTIES(xyuv PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(xyuv
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/xyuv/include>
    PUBLIC $<INSTALL_INTERFACE:xyuv/include>
    PRIVATE thirdparty/rapidjson/include
)

install(DIRECTORY xyuv/include/
        DESTINATION include
        FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
        PATTERN xyuv/include/xyuv/external EXCLUDE)
install(DIRECTORY xyuv/include/
        DESTINATION include
        FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
install(DIRECTORY formats/
        DESTINATION share/xyuv
        FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
install(TARGETS xyuv
        EXPORT XYUV
        DESTINATION lib)
install(EXPORT XYUV
        NAMESPACE XYUV_
        DESTINATION lib/xyuv)

if(PNG_FOUND)
    target_include_directories(xyuv
            PUBLIC ${PNG_INCLUDE_DIRS})
    target_link_libraries(xyuv
            PUBLIC ${PNG_LIBRARIES})
    target_compile_definitions(xyuv
            PUBLIC ${PNG_DEFINITIONS})
    add_definitions(-DUSE_LIBPNG=1)
    install(FILES xyuv/include/xyuv/external/libpng_wrapper.h
            DESTINATION include/xyuv/external/libpng_wrapper.h
            PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
endif(PNG_FOUND)


if (ImageMagick_FOUND)
    target_include_directories(xyuv
        PUBLIC ${ImageMagick_INCLUDE_DIRS})
    target_link_libraries(xyuv
            PUBLIC ${ImageMagick_LIBRARIES})
    add_definitions(-DUSE_IMAGEMAGICK=1)
    install(FILES xyuv/include/xyuv/external/magick_wrapper.h
            DESTINATION include/xyuv/external/magick_wrapper.h
            PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
endif(ImageMagick_FOUND)

#Add subdirectories for other targets.
add_subdirectory(thirdparty)
add_subdirectory(testing)
add_subdirectory(utilities)
