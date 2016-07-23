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

#pragma once

#include <xyuv/rgb_image.h>
#include <string>
#include <xyuv/structures/color.h>

namespace xyuv {

    //! \brief Internal data structure.
    struct libpng_wrapper_internal_data_struct;

    //! Simple reference implementation of the xyuv::rgb_image interface based on libpng.
    //! \details Supports loading and storing png images.
    class libpng_wrapper : public rgb_image {
    public:
        //! \brief Enum signalling the requested number of bits per channel
        enum BitDepth {
            BITS_8,
            BITS_16,
        };

        //! \brief Default constructor,
        libpng_wrapper();

        //! \brief Load a image from a png file.
        explicit libpng_wrapper(const std::string & pngfile);

        //! \brief Create an empty image. The image will be transparent BLACK.
        libpng_wrapper(uint32_t width, uint32_t height, BitDepth bits = BITS_8);

        ~libpng_wrapper();
        libpng_wrapper(const libpng_wrapper & other);
        libpng_wrapper(libpng_wrapper && other);
        libpng_wrapper& operator=(libpng_wrapper &&other);
        libpng_wrapper& operator=(const libpng_wrapper &other);

        //! Load an image from a png file.
        void load_png_from_file(const std::string & pngfile);
        //! Store image to png file.
        void save_png_to_file(const std::string &pngfile);

        //! Get color at coordinate (x,y). Unsafe to call outside image boundaries.
        rgb_color get_pixel(uint32_t x, uint32_t y) const;
        //! Set color at coordinate (x,y). Unsafe to call outside image boundaries.
        void set_pixel(uint32_t x, uint32_t y, const rgb_color &color);

        //! Return the height of the image.
        uint32_t rows() const;

        //! Return the width of the image.
        uint32_t columns() const;

        /** \brief Implementation of xyuv::rgb_image interface.
         * \details *this will be resized to the input yuv_image. Retaining bit depth if already set, otherwise the new
         *   image will have bit depth 8 bits.
         */
        virtual void xyuv_from_yuv_image_444(const xyuv::yuv_image &yuv_image_444,
                                             const xyuv::conversion_matrix &conversion_matrix) override;

        //! \brief Implementation of xyuv::rgb_image interface.
        virtual yuv_image xyuv_to_yuv_image_444(const xyuv::conversion_matrix &conversion_matrix) const override;

    private:
        libpng_wrapper_internal_data_struct * data = nullptr;
    };
}