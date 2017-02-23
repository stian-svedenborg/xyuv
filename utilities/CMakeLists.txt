
set(XYUV_UTILS_COMMON_SOURCES
        helpers.h
        helpers_read_frame.cpp
        helpers_add_header.cpp
        helpers_write_frame.cpp
        console_width.h
        console_width.cpp
        hex_reader.cpp helpers_console_utils.cpp helpers_misc.cpp)

if(ImageMagick_FOUND)
list(APPEND XYUV_UTILS_COMMON_SOURCES
        external/magick_format_rw.cpp
        external/magick_format_rw.h
        )
endif(ImageMagick_FOUND)
if(PNG_FOUND)
list(APPEND XYUV_UTILS_COMMON_SOURCES
        external/libpng_format_rw.cpp
        external/libpng_format_rw.h
        )
endif(PNG_FOUND)

set(XYUV_HEADER_SOURCES
        ${XYUV_UTILS_COMMON_SOURCES}
        xyuv_header_parse_args.cpp
        xyuv_header_run.cpp
        XYUVHeader.h
        XYUVHeader.cpp)

set(XYUV_ENCODE_SOURCES
        ${XYUV_UTILS_COMMON_SOURCES}
        xyuv-encode.cpp
        )


add_executable(xyuv-header ${XYUV_HEADER_SOURCES})
add_executable(xyuv-encode ${XYUV_ENCODE_SOURCES})

target_include_directories(xyuv-header
    PUBLIC ${xyuv.PUBLIC}
)
target_include_directories(xyuv-encode
    PUBLIC ${xyuv.PUBLIC}
)

target_link_libraries(xyuv-header
    PUBLIC xyuv)
target_link_libraries(xyuv-encode
    PUBLIC xyuv)

target_compile_options(xyuv-header
    PUBLIC -std=c++11
)
target_compile_options(xyuv-encode
    PUBLIC -std=c++11
)

target_compile_definitions(xyuv-header
    PRIVATE INSTALL_FORMATS_PATH="${CMAKE_INSTALL_PREFIX}/share/xyuv")
target_compile_definitions(xyuv-encode
    PRIVATE INSTALL_FORMATS_PATH="${CMAKE_INSTALL_PREFIX}/share/xyuv")


add_dependencies(xyuv-header xyuv)
add_dependencies(xyuv-encode xyuv)

if(PNG_FOUND)

    target_include_directories(xyuv-header
            PUBLIC ${PNG_INCLUDE_DIRS})
    target_link_libraries(xyuv-header
            PUBLIC ${PNG_LIBRARIES})
    target_compile_definitions(xyuv-header
            PUBLIC ${PNG_DEFINITIONS})

    target_include_directories(xyuv-encode
            PUBLIC ${PNG_INCLUDE_DIRS})
    target_link_libraries(xyuv-encode
            PUBLIC ${PNG_LIBRARIES})
    target_compile_definitions(xyuv-encode
            PUBLIC ${PNG_DEFINITIONS})

endif(PNG_FOUND)

if (ImageMagick_FOUND)
    target_include_directories(xyuv-header
            PUBLIC ${ImageMagick_INCLUDE_DIRS})
    target_link_libraries(xyuv-header
            PUBLIC ${ImageMagick_LIBRARIES})

    target_include_directories(xyuv-encode
            PUBLIC ${ImageMagick_INCLUDE_DIRS})
    target_link_libraries(xyuv-encode
            PUBLIC ${ImageMagick_LIBRARIES})
endif(ImageMagick_FOUND)

#Installation section
install(TARGETS xyuv-header DESTINATION bin)
install(TARGETS xyuv-encode DESTINATION bin)
