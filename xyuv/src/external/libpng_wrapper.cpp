/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Stian Valentin Svedenborg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <xyuv/external/libpng_wrapper.h>
#include <xyuv/yuv_image.h>
#include <xyuv.h>
#include <stdexcept>
#include <png.h>
#include <cstdlib>
#include <cstdio>
#include <xyuv/structures/color.h>
#include "../rgb_conversion.h"
#include "../assert.h"
#include "../utility.h"

namespace xyuv {

    struct libpng_wrapper_internal_data_struct {
        png_bytep image_data = nullptr;
        png_bytepp row_pointers = nullptr;

        png_uint_32 width = 0;
        png_uint_32 height = 0;
        int bit_depth = 0;
        int channels = 0;
    };

    static libpng_wrapper_internal_data_struct *create_internal_data() {
        return new libpng_wrapper_internal_data_struct;
    }

    static void destroy_internal_data(libpng_wrapper_internal_data_struct **ptr) {
        if (*ptr) {
            delete[] (*ptr)->image_data;
            delete[] (*ptr)->row_pointers;
            delete *ptr;
            *ptr = nullptr;
        }
    }

    libpng_wrapper::libpng_wrapper() : data(nullptr) {
    }


    libpng_wrapper::libpng_wrapper(uint32_t width, uint32_t height, libpng_wrapper::BitDepth bits) {
        try {
            this->data = create_internal_data();
            this->data->width = width;
            this->data->height = height;
            this->data->bit_depth = (bits == BITS_8 ? 8 : 16);
            this->data->channels = 4;
            this->data->row_pointers = new png_bytep[this->data->height]();
            png_uint_32 row_bytes = this->data->channels * this->data->width * this->data->bit_depth / 8;
            this->data->image_data = new png_byte[row_bytes * this->data->height]();
            for (int y = 0; y < this->data->height; y++) {
                this->data->row_pointers[y] = this->data->image_data + y * row_bytes;
            }
        } catch(...) {
            destroy_internal_data(&this->data);
            throw;
        }
    }


    libpng_wrapper::libpng_wrapper(const std::string &pngfile) : libpng_wrapper() {
        load_png_from_file(pngfile);
    }

    libpng_wrapper::~libpng_wrapper() {
        destroy_internal_data(&this->data);
    }


    libpng_wrapper::libpng_wrapper(const libpng_wrapper &other) : libpng_wrapper() {
        if (other.data) {
            this->data = create_internal_data();

            // Copy all and reinit pointer.
            *this->data = *other.data;

            png_uint_32 row_bytes = this->data->channels * this->data->width * this->data->bit_depth/8;
            png_uint_32 image_size = row_bytes * this->data->height;
            this->data->image_data = new png_byte[image_size]();
            memcpy(this->data->image_data, other.data->image_data, image_size);
            this->data->row_pointers = new png_bytep[this->data->height];
            for (png_uint_32 i = 0; i < this->data->height; i++) {
                this->data->row_pointers[i] = this->data->image_data + i*row_bytes;
            }
        }
    }

    libpng_wrapper::libpng_wrapper(libpng_wrapper &&other) {
        std::swap(this->data, other.data);
    }


    libpng_wrapper &libpng_wrapper::operator=(libpng_wrapper &&other) {
        std::swap(this->data, other.data);
        return *this;
    }


    libpng_wrapper &libpng_wrapper::operator=(const libpng_wrapper &other_) {
        libpng_wrapper other(other_);
        std::swap(this->data, other.data);
        return *this;
    }

    void libpng_wrapper::load_png_from_file(const std::string &pngfile) {
        FILE *fp = nullptr;
        libpng_wrapper_internal_data_struct *internal_data = nullptr;
        png_structp png_ptr = nullptr;
        png_infop info_ptr = nullptr;
        try {
            fp = std::fopen(pngfile.c_str(), "rb");

            if (!fp) {
                throw std::runtime_error("Could not open file " + pngfile);
            }

            const int HEADER_PEEK_SIZE = 8;
            png_byte header[HEADER_PEEK_SIZE] = {};

            if (fread(header, 1, HEADER_PEEK_SIZE, fp) != HEADER_PEEK_SIZE ||
                png_sig_cmp(header, 0, HEADER_PEEK_SIZE) != 0) {
                throw std::runtime_error("File '" + pngfile + "' is not a png file.");
            }

            // File open correctly, initialize png structures.
            internal_data = create_internal_data();

            png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                                            nullptr, nullptr);

            if (!png_ptr) {
                throw std::runtime_error("libpng internal error.");
            }

            info_ptr = png_create_info_struct(png_ptr);

            if (!info_ptr) {
                throw std::runtime_error("libpng internal error.");
            }

            if (setjmp(png_jmpbuf(png_ptr))) {
                throw std::runtime_error("libpng internal error.");
            }

            // ----

            png_init_io(png_ptr, fp);
            png_set_sig_bytes(png_ptr, HEADER_PEEK_SIZE);

            png_read_info(png_ptr, info_ptr);

            internal_data->width = png_get_image_width(png_ptr, info_ptr);
            internal_data->height = png_get_image_height(png_ptr, info_ptr);
            int color_type = png_get_color_type(png_ptr, info_ptr);
            internal_data->bit_depth = png_get_bit_depth(png_ptr, info_ptr);

            if (internal_data->bit_depth == 16)
                png_set_swap(png_ptr);

            if (color_type == PNG_COLOR_TYPE_PALETTE)
                png_set_palette_to_rgb(png_ptr);

            if (color_type == PNG_COLOR_TYPE_GRAY && internal_data->bit_depth < 8)
                png_set_expand_gray_1_2_4_to_8(png_ptr);

            if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
                png_set_tRNS_to_alpha(png_ptr);

            if (color_type == PNG_COLOR_TYPE_RGB ||
                color_type == PNG_COLOR_TYPE_GRAY ||
                color_type == PNG_COLOR_TYPE_PALETTE)
                png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

            if (color_type == PNG_COLOR_TYPE_GRAY ||
                color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
                png_set_gray_to_rgb(png_ptr);

            png_read_update_info(png_ptr, info_ptr);

            internal_data->row_pointers = new png_bytep[internal_data->height];

            const png_uint_32 row_bytes = png_get_rowbytes(png_ptr, info_ptr);
            internal_data->image_data = new png_byte[internal_data->height * row_bytes];

            for (int y = 0; y < internal_data->height; y++) {
                internal_data->row_pointers[y] = internal_data->image_data + y * row_bytes;
            }

            png_read_image(png_ptr, internal_data->row_pointers);
            fclose(fp);
            fp = nullptr;

            internal_data->channels = png_get_channels(png_ptr, info_ptr);

            // PNG loaded successfully, overwrite internal data.
            if (this->data) {
                destroy_internal_data(&this->data);
            }
            std::swap(this->data, internal_data);


            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        }
        catch (...) {
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            destroy_internal_data(&internal_data);
            if (fp) std::fclose(fp);
            throw;
        }
    }

    void libpng_wrapper::save_png_to_file(const std::string &pngfile) {
        FILE *fp = nullptr;
        png_structp png_ptr = nullptr;
        png_infop info_ptr = nullptr;

        try {
            fp = fopen(pngfile.c_str(), "wb");

            if (!fp) {
                throw std::runtime_error("Could not open file " + pngfile);
            }

            png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if (!png_ptr) {
                throw std::runtime_error("libpng internal error.");
            }

            info_ptr = png_create_info_struct(png_ptr);

            if (!info_ptr) {
                throw std::runtime_error("libpng internal error.");
            }

            if (setjmp(png_jmpbuf(png_ptr))) {
                throw std::runtime_error("libpng internal error.");
            }

            png_init_io(png_ptr, fp);

            png_set_IHDR(
                    png_ptr,
                    info_ptr,
                    this->data->width, this->data->height,
                    this->data->bit_depth,
                    PNG_COLOR_TYPE_RGBA,
                    PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT,
                    PNG_FILTER_TYPE_DEFAULT
            );
            png_write_info(png_ptr, info_ptr);
            png_write_image(png_ptr, this->data->row_pointers);
            png_write_end(png_ptr, NULL);

            png_destroy_write_struct(&png_ptr, &info_ptr);

            fclose(fp);
            fp = nullptr;

        } catch (...) {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            if (fp) fclose(fp);
        }
    }

    void libpng_wrapper::xyuv_from_yuv_image_444(const xyuv::yuv_image &yuv_image_444,
                                                 const xyuv::conversion_matrix &conversion_matrix) {
        // Check if we need to reallocate image.
        if (this->data == nullptr || !(yuv_image_444.image_h <= this->data->height && yuv_image_444.image_w <= this->data->width)) {
            *this = std::move(libpng_wrapper(yuv_image_444.image_w, yuv_image_444.image_h, (this->data && this->data->bit_depth == 16) ? BITS_16 : BITS_8));
        } else {
            // Overwrite width + height for the case where we have not reallocated.
            this->data->width = yuv_image_444.image_w;
            this->data->height = yuv_image_444.image_h;
        }

        bool has_y = !yuv_image_444.y_plane.empty();
        bool has_u = !yuv_image_444.u_plane.empty();
        bool has_v = !yuv_image_444.v_plane.empty();
        bool has_a = !yuv_image_444.a_plane.empty();

        xyuv::yuv_color yuv_color;
        xyuv::rgb_color rgb_color;

        // Load image
        if (this->data->bit_depth == 16) {
            using current_ch_type = png_uint_16;
            constexpr pixel_quantum channel_max = static_cast<pixel_quantum>(std::numeric_limits<current_ch_type>::max());

            for (png_uint_32 row = 0; row < this->data->height; row++) {
                current_ch_type *pixel_row = reinterpret_cast<current_ch_type *>(this->data->row_pointers[row]);
                for (png_uint_32 column = 0; column < this->data->width; column++) {


                    if (has_y) yuv_color.y = yuv_image_444.y_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row));
                    if (has_u) yuv_color.u = yuv_image_444.u_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row));
                    if (has_v) yuv_color.v = yuv_image_444.v_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row));
                    if (has_a) yuv_color.a = yuv_image_444.a_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row));

                    xyuv::to_rgb(&rgb_color, yuv_color, conversion_matrix, has_y, has_u, has_v );

                    switch (this->data->channels) {
                        case 4:
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.r*channel_max));
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.g*channel_max));
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.b*channel_max));
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.a*channel_max));
                            break;
                        case 3:
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.r*channel_max));
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.g*channel_max));
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.b*channel_max));
                            break;
                        case 2:
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.r*channel_max));
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.g*channel_max));
                            break;
                        case 1:
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.r*channel_max));
                            break;
                        default: break;
                    }
                }
            }
        } else {
            using current_ch_type = png_byte;
            constexpr pixel_quantum channel_max = static_cast<pixel_quantum>(std::numeric_limits<current_ch_type>::max());

            for (png_uint_32 row = 0; row < this->data->height; row++) {
                current_ch_type *pixel_row = reinterpret_cast<current_ch_type *>(this->data->row_pointers[row]);
                for (png_uint_32 column = 0; column < this->data->width; column++) {


                    if (has_y) yuv_color.y = yuv_image_444.y_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row));
                    if (has_u) yuv_color.u = yuv_image_444.u_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row));
                    if (has_v) yuv_color.v = yuv_image_444.v_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row));
                    if (has_a) yuv_color.a = yuv_image_444.a_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row));

                    xyuv::to_rgb(&rgb_color, yuv_color, conversion_matrix, has_y, has_u, has_v );

                    switch (this->data->channels) {
                        case 4:
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.r*channel_max));
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.g*channel_max));
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.b*channel_max));
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.a*channel_max));
                            break;
                        case 3:
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.r*channel_max));
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.g*channel_max));
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.b*channel_max));
                            break;
                        case 2:
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.r*channel_max));
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.g*channel_max));
                            break;
                        case 1:
                            (*(pixel_row++)) = static_cast<current_ch_type>(xyuv::clamp(0, channel_max, rgb_color.r*channel_max));
                            break;
                        default: break;
                    }
                }
            }
        }

    }

    yuv_image xyuv::libpng_wrapper::xyuv_to_yuv_image_444(const xyuv::conversion_matrix &conversion_matrix) const {
        
        if (!data) {
            throw std::logic_error("No image loaded.");
        }
        xyuv::yuv_image out = xyuv::create_yuv_image_444(static_cast<uint32_t>(this->data->width),
                                                         static_cast<uint32_t>(this->data->height));

        pixel_quantum rgba[4] = {};

        if (this->data->bit_depth == 16) {
            constexpr pixel_quantum channel_max = static_cast<pixel_quantum>(std::numeric_limits<png_uint_16>::max());

            for (png_uint_32 row = 0; row < this->data->height; row++) {
                png_uint_16 *pixel_row = reinterpret_cast<png_uint_16 *>(this->data->row_pointers[row]);
                for (png_uint_32 column = 0; column < this->data->width; column++) {
                    for (uint32_t channel = 0; channel < this->data->channels; channel++) {
                        rgba[channel++] = (*(pixel_row++)) / channel_max;
                    }

                    xyuv::rgb_color rgbColor(rgba[0], rgba[1], rgba[2], rgba[3]);
                    xyuv::yuv_color yuvColor;
                    xyuv::to_yuv(&yuvColor, rgbColor, conversion_matrix);

                    out.y_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row)) = yuvColor.y;
                    out.u_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row)) = yuvColor.u;
                    out.v_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row)) = yuvColor.v;
                    out.a_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row)) = yuvColor.a;
                }
            }
        } else {
            constexpr pixel_quantum channel_max = static_cast<pixel_quantum>(std::numeric_limits<png_byte>::max());

            XYUV_ASSERT(this->data->bit_depth == 8);
            for (png_uint_32 row = 0; row < this->data->height; row++) {
                png_bytep pixel_row = reinterpret_cast<png_bytep>(this->data->row_pointers[row]);
                for (png_uint_32 column = 0; column < this->data->width; column++) {
                    for (uint32_t channel = 0; channel < this->data->channels; channel++) {
                        rgba[channel] = (*(pixel_row++)) / channel_max;
                    }

                    xyuv::rgb_color rgbColor(rgba[0], rgba[1], rgba[2], rgba[3]);
                    xyuv::yuv_color yuvColor;
                    xyuv::to_yuv(&yuvColor, rgbColor, conversion_matrix);

                    out.y_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row)) = yuvColor.y;
                    out.u_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row)) = yuvColor.u;
                    out.v_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row)) = yuvColor.v;
                    out.a_plane.at(static_cast<uint32_t>(column), static_cast<uint32_t>(row)) = yuvColor.a;
                }
            }
        }
        return out;
    }


    rgb_color libpng_wrapper::get_pixel(uint32_t x, uint32_t y) const {
        pixel_quantum rgba[4] = {};
        if (this->data->bit_depth == 16) {
            constexpr pixel_quantum channel_max = static_cast<pixel_quantum>(std::numeric_limits<png_uint_16>::max());
            png_uint_16p pixel_row = reinterpret_cast<png_uint_16p>(this->data->row_pointers[y]) + this->data->channels*x;;
            for (uint32_t channel = 0; channel < this->data->channels; channel++) {
                rgba[channel++] = (*(pixel_row++)) / channel_max;
            }
            return xyuv::rgb_color(rgba[0], rgba[1], rgba[2], rgba[3]);
        }
        else {
            constexpr pixel_quantum channel_max = static_cast<pixel_quantum>(std::numeric_limits<png_byte>::max());
            png_bytep pixel_row = reinterpret_cast<png_bytep>(this->data->row_pointers[y]) + this->data->channels*x;;
            for (uint32_t channel = 0; channel < this->data->channels; channel++) {
                rgba[channel++] = (*(pixel_row++)) / channel_max;
            }
            return xyuv::rgb_color(rgba[0], rgba[1], rgba[2], rgba[3]);
        }
    }

    void libpng_wrapper::set_pixel(uint32_t x, uint32_t y, const rgb_color &color) {
        pixel_quantum _rgba[4] = {color.r, color.g, color.b, color.a};
        pixel_quantum *rgba = _rgba;

        if (this->data->bit_depth == 16) {
            constexpr pixel_quantum channel_max = static_cast<pixel_quantum>(std::numeric_limits<png_uint_16>::max());
            png_uint_16p pixel_ptr = reinterpret_cast<png_uint_16p>(this->data->row_pointers[y]) + this->data->channels*x;

            switch (this->data->channels) {
                case 4: *(pixel_ptr++) = static_cast<png_uint_16>(xyuv::clamp(0, channel_max, *(rgba++)*channel_max));
                case 3: *(pixel_ptr++) = static_cast<png_uint_16>(xyuv::clamp(0, channel_max, *(rgba++)*channel_max));
                case 2: *(pixel_ptr++) = static_cast<png_uint_16>(xyuv::clamp(0, channel_max, *(rgba++)*channel_max));
                case 1: *(pixel_ptr++) = static_cast<png_uint_16>(xyuv::clamp(0, channel_max, *(rgba++)*channel_max));
                default: break;
            }
        }
        else {
            {
                constexpr pixel_quantum channel_max = static_cast<pixel_quantum>(std::numeric_limits<png_byte>::max());
                png_bytep pixel_ptr = reinterpret_cast<png_bytep>(this->data->row_pointers[y]) + this->data->channels*x;

                switch (this->data->channels) {
                    case 4: *(pixel_ptr++) = static_cast<png_byte>(xyuv::clamp(0, channel_max, *(rgba++)*channel_max));
                    case 3: *(pixel_ptr++) = static_cast<png_byte>(xyuv::clamp(0, channel_max, *(rgba++)*channel_max));
                    case 2: *(pixel_ptr++) = static_cast<png_byte>(xyuv::clamp(0, channel_max, *(rgba++)*channel_max));
                    case 1: *(pixel_ptr++) = static_cast<png_byte>(xyuv::clamp(0, channel_max, *(rgba++)*channel_max));
                    default: break;
                }
            }
        }
    }


    uint32_t libpng_wrapper::rows() const {
        return data ? 0 : static_cast<uint32_t >(this->data->height);
    }

    uint32_t libpng_wrapper::columns() const {
        return data ? 0 : static_cast<uint32_t >(this->data->width);
    }


}