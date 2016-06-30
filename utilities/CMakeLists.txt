
set(XYUV_HEADER_SOURCES
        XYUVHeader.h
        XYUVHeader.cpp
        xyuv_header_parse_args.cpp
        xyuv_header_run.cpp
        xyuv_header_write_frame.cpp
        xyuv_header_read_frame.cpp
        console_width.h
        console_width.cpp
        hex_reader.cpp)

if(ImageMagick_FOUND)
list(APPEND XYUV_HEADER_SOURCES
        external/magick_format_rw.cpp
        external/magick_format_rw.h
        )
endif(ImageMagick_FOUND)
if(PNG_FOUND)
list(APPEND XYUV_HEADER_SOURCES
        external/libpng_format_rw.cpp
        external/libpng_format_rw.h
        )
endif(PNG_FOUND)

add_executable(xyuv-header ${XYUV_HEADER_SOURCES})

target_include_directories(xyuv-header
    PUBLIC ${xyuv.PUBLIC}
)
target_link_libraries(xyuv-header
        PUBLIC xyuv)
target_compile_options(xyuv-header
    PUBLIC -std=c++11
)

target_compile_definitions(xyuv-header
    PRIVATE INSTALL_FORMATS_PATH="${CMAKE_INSTALL_PREFIX}/share/xyuv")

add_dependencies(xyuv-header xyuv)

if(PNG_FOUND)
    target_include_directories(xyuv-header
            PUBLIC ${PNG_INCLUDE_DIRS})
    target_link_libraries(xyuv-header
            PUBLIC ${PNG_LIBRARIES})
    target_compile_definitions(xyuv-header
            PUBLIC ${PNG_DEFINITIONS})
endif(PNG_FOUND)

if (ImageMagick_FOUND)
    target_include_directories(xyuv-header
            PUBLIC ${ImageMagick_INCLUDE_DIRS})
    target_link_libraries(xyuv-header
            PUBLIC ${ImageMagick_LIBRARIES})
endif(ImageMagick_FOUND)

#Installation section
install(TARGETS xyuv-header DESTINATION bin)
