#include <Magick++/Image.h>
#include <fstream>
#include <xyuv/imagemagick/magick_wrapper.h>
#include <xyuv/large_buffer.h>
#include <xyuv/frame.h>
#include <iostream>
#include "XYUVHeader.h"

std::string get_suffix(const std::string & path) {
    return path.substr(path.rfind('.', std::string::npos), std::string::npos);
}

static void write_frame_using_image_magick(const xyuv::frame &frame, const std::string & out_filename) {
    Magick::Image image(Magick::Geometry(1,1,0,0), Magick::ColorRGB(0.0,0.0,0.0));

    xyuv::magick_wrapper wrapper(image);
    xyuv::write_frame_to_rgb_image(&wrapper, frame);

    image.write(out_filename);
}

static void write_frame_using_xyuv(const xyuv::frame & frame, const std::string & out_filename) {
    std::ofstream fout(out_filename, std::ios::binary);
    if (!fout) {
        throw std::runtime_error("Could not open output file: '" + out_filename + "' for writing");
    }
    xyuv::write_frame(fout, frame);
}

static void write_frame_raw(const xyuv::frame & frame, const std::string & out_filename) {
    std::ofstream fout(out_filename, std::ios::binary);
    if (!fout) {
        throw std::runtime_error("Could not open output file: '" + out_filename + "' for writing");
    }
    xyuv::write_large_buffer(fout, reinterpret_cast<const char*>(frame.data.get()), frame.format.size);
}

static void write_frame_hex(const xyuv::frame &, const std::string & out_filename) {
    throw std::runtime_error("Hex file support not yet implemented.");
}

void XYUVHeader::WriteFrame(const xyuv::frame & frame, const std::string & out_filename) {
    static std::map<std::string, std::function<void(const xyuv::frame &, const std::string &)>> map {
            {".xyuv", write_frame_using_xyuv },
            {".hex", write_frame_hex },
            {".bin", write_frame_raw },
            {".raw", write_frame_raw },
            {".yuv", write_frame_raw },
            {".png", write_frame_using_image_magick },
            {".jpg", write_frame_using_image_magick },
            {".gif", write_frame_using_image_magick },
            {".tga", write_frame_using_image_magick },
            {".bmp", write_frame_using_image_magick },
    };

    std::string suffix = get_suffix(out_filename);

    // Make it lowercase.
    for (char & c : suffix ) {
        c = static_cast<char>(std::tolower(c));
    }

    // If the suffix is in out map, use it
    auto it = map.find(suffix);
    if (it != map.end()) {
        (it->second)(frame, out_filename);
    }
    else {
        std::cout << "[Warning]: Unrecognized file suffix '" + suffix + "' trying to pass it to imagemagick." << std::endl;
        write_frame_using_image_magick(frame, out_filename);
    }
}
