
add_executable(xyuv-header
    XYUVHeader.h
    XYUVHeader.cpp
    xyuv_header_parse_args.cpp
    xyuv_header_run.cpp
    xyuv_header_write_frame.cpp
    xyuv_header_read_frame.cpp
    console_width.h
    console_width.cpp
)

target_include_directories(xyuv-header
    PUBLIC ${xyuv.PUBLIC}
)
target_link_libraries(xyuv-header xyuv)
target_compile_options(xyuv-header
    PUBLIC -std=c++11
)
add_dependencies(xyuv-header xyuv)

# If we have compiled support for ImageMagick also include that.
if (USE_IMAGEMAGICK_INTEGRATION)
    # Resolve dependencies
    find_package(ImageMagick REQUIRED COMPONENTS Magick++)
    message("Found ImageMagick headers at ${ImageMagick_INCLUDE_DIRS}")

    target_include_directories(xyuv-header
        PUBLIC ${ImageMagick_INCLUDE_DIRS}
    )

    target_link_libraries(xyuv-header ${ImageMagick_LIBRARIES})

    target_compile_options(xyuv-header
        PUBLIC -DMAGICKCORE_QUANTUM_DEPTH=${MAGICKCORE_QUANTUM_DEPTH}
        PUBLIC -DMAGICKCORE_HDRI_ENABLE=${MAGICKCORE_HDRI_ENABLE}
        PUBLIC -DXYUV_USE_IMAGEMAGICK_INTEGRATION
    )

endif (USE_IMAGEMAGICK_INTEGRATION)

#Installation section
install (TARGETS xyuv-header DESTINATION bin)