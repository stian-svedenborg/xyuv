/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Stian Valentin Svedenborg
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

#include <cstdint>
#include <iosfwd>

//! \file High level interface to libxyuv.

//! \namespace xyuv
//! Primary namespace of the libxyuv.
namespace xyuv {

// High level classes and structs.
// The declaration of these structures can be found under include/xyuv/structures

//! A choma siting struct holds the necessary parameters to describe a sub-sampling operation.
//! e.g. YUV-420 or YUV-422
struct chroma_siting;

//! A conversion matrix holds the parameters to describe an RGB <-> YUV conversion.
struct conversion_matrix;

//! A format template holds the generic parameters to describe a specific pixel-format.
//! i.e. Formulas to describe line stride, pixel-packing etc.
struct format_template;

//! A format describes the pixel-format and layout of a single frame. A format holds all the meta-data for a frame,
//! everything from frame dimensions to pixel-packing, chroma-siting and rgb conversion.
//! A format is usually created from combining image_w, and image_h, a format-template, a conversion matrix and a
//! chroma siting descriptor.
struct format;

//! A frame is the realisation of a format together with raw-pixel data.
struct frame;

//! A yuv image is the intermediate representation of an image. Stored in a high precision internal format.
struct yuv_image;

//! An rgb image is an interface-class to simplify interaction of libxyuv to other image libraries.
class rgb_image;

///////////////////////////////////////////
// High level interface
///////////////////////////////////////////


//! \brief Initialize a xyuv::frame from a xyuv::format and raw data.
//! \details This will create a new frame object to hold an image of the format specified by \a format.
//!
//! \param [in] format format-struct describing the pixel-layout of the raw date.
//! \param [in] raw_data, a pointer to the raw data which should be copied into the frame. If raw_data is nullptr, then
//! no data is copied and the pixel-data in the returned frame is set to all zero.
//! \param [in] raw_data_size the number of bytes to copy from the data pointed to by \a raw_data. If \a raw_data is
//! nullptr, then this argument is ignored. raw_data_size must be less or equal to the size of the frame as given in \a
//! format.
//! \returns A fully constructed frame object with format.size bytes allocated in it's data field.
xyuv::frame create_frame(
        const xyuv::format &format,
        const uint8_t *raw_data,
        uint64_t raw_data_size
);

//! \brief Initialize a format struct from a format_template.
//! \details This will combine a xyuv::format_template, a xyuv::conversion_template and a xyuv::chroma_siting to create
//! a format struct describing the pixel-format of a single image.
//!
//! \param [in] width of the described image.
//! \param [in] height of the described image.
//! \param [in] format_template template to describe the pixel layout and format of the described image. Usually
//!             loaded from a text description through the configuration manager.
//! \param [in] conversion_matrix describing YUV <-> RGB conversion.
//! \returns valid format-struct.
xyuv::format create_format(
        uint32_t width,
        uint32_t height,
        const xyuv::format_template &format_template,
        const xyuv::conversion_matrix &conversion_matrix,
        const xyuv::chroma_siting &chroma_siting
);

//! \brief Write a frame to an output stream.
//!
//! \details This will write a frame to a C++ standard output stream in an architecture neutral fashion. i.e. frame may be
//! written to a file or sent over a network.
//! \warning \a ostream should be opened in binary mode, otherwise the resulting image may be invalid on certain platforms.
//! \param [out] ostream C++ standard library output stream which to write the frame.
//! \param [in] a valid frame object to serialise.
void write_frame(
        std::ostream &ostream,
        const xyuv::frame &frame
);

//! \brief Read a frame from an input stream.
//!
//! \details This will read a frame-struct from a C++ standard input stream. The frame must have been previously written
//! using xyuv::write_frame().
//! \warning \a istream should be opened in binary mode.
//! \param [in] istream C++ standard library binary input stream from which to load the frame.
//! \param [out] frame pointer to an object object where the values should be stored.
void read_frame(
        xyuv::frame *frame,
        std::istream &istream
);

//! \brief Convert a frame to a new format.
//!
//! \details This function will convert a frame to a frame with a new format.
//! This allows you to change the pixel-layout, size or sub-sampling, anything specified in the format struct.
//! \param [in] frame_in frame to be converted.
//! \param [in] new_format of the returned frame.
//! \returns a new xyuv::frame with the new pixel data of \a frame_in, now converted to the new format.
xyuv::frame convert_frame(const xyuv::frame &frame_in, const xyuv::format &new_format);

//! \brief Read a frame through the xyuv::rgb_image interface.
//!
//! \details This function will load a frame through the xyuv::rgb_image interface and encode it in the supplied format.
//! \param [in] rgbImage_in rgb_image to read.
//! \param [in] new_format target format for the returned frame.
//! \returns A new xyuv::frame with the new pixel data of \a rgbImage_in, now converted to the new format.
xyuv::frame read_frame_from_rgb_image(const rgb_image &rgbImage_in, const xyuv::format &new_format);

//! \brief Write a frame using the xyuv::rgb_image interface.
//!
//! \details This function will write a frame to an RGB image using the xyuv::rgb_image interface.
//! \param [out] rgbImage_out Target to decode the frame to.
//! \param [in] frame_in frame to write.
void write_frame_to_rgb_image(rgb_image *rgbImage_out, const xyuv::frame &frame_in);

// Mid-level image manipulation functions:

//! \brief Decode a frame to a xyuv::yuv_image.
//!
//! \details This will unpack the pixel-data in the frame, but will leave any sub-sampling unchanged. i.e. if you have a
//! 420 sub-sampled frame, your yuv_image will also be 420 sub-sampled.
//! \info The returned yuv_image has not concept of rgb<->yuv conversion, so users will have to maintain this separately.
//!       See the xyuv::yuv_imgage and xyuv::conversion_matrix for details.
//! \param [in] frame_in frame to decode.
//! \returns yuv_image containing the decoded frame.
yuv_image decode_frame(const xyuv::frame &frame_in);

//! \brief Encode a xyuv::yuv_image to a xyuv::frame.
//!
//! \details Encode the image data in the yuv_image into a frame using the supplied format.
//! \param [in] yuva image data to write to the frame.
//! \param [in] format target format of the frame.
//! \returns A new xyuv::frame with the new pixel data of \a yuva, now converted to the new format.
 xyuv::frame encode_frame(const yuv_image &yuva, const xyuv::format &format);

//! \brief Upsample a yuv_image to full resolution.
//!
//! \details This will return a copy of the input image that is <b>not</b> subsampled, i.e. has one sample per channel, per pixel.
//! \info Currently this will simply copy the single sample into each of it's corresponding pixels, in future releases
//!       it may also include interpolation.
//! \param [in] yuva_in input yuv_image.
//! \returns A copy of the subsampled image up-sampled to full resolution.
//! \todo Split mid-level tell-don't-ask functions. And move the low-level interface internally.
yuv_image up_sample(const yuv_image &yuva_in);

//! \brief Downsample a yuv_image.
//!
//! \details Subsample a yuv_image using the supplied chroma_siting. The input yuv_image may be of any sub_sampling,
//!          the image is upsampled if needed.
//! \info Downsampling is calculated as the weighted average of the (<= four) pixels (inside the block) closest to the sampling point.
//! \param [in] yuva_in image to convert.
//! \param [in] siting target chroma_siting.
//! \todo Split mid-level tell-don't-ask functions. And move the low-level interface internally.
yuv_image down_sample(const yuv_image &yuva_in, const chroma_siting &siting);

//! \brief Write a frame using the xyuv::rgb_image interface.
//!
//! \details This function will write a yuv_image to an RGB image using the xyuv::rgb_image interface.
//! \param [out] rgbImage_out Target to decode the frame.
//! \param [in] yuva_in yuva_image to write.
//! \param [in] conversion conversion_matrix describing the yuv color space.
void yuv_image_to_rgb(rgb_image *rgbImage_out, const yuv_image &yuva_444, const xyuv::conversion_matrix &conversion);

//! \brief Read a yuv_image through the xyuv::rgb_image interface.
//!
//! \details This function will load a yuv_image through the xyuv::rgb_image using the supplied conversion_matrix.
//! \param [in] rgbImage_in rgb_image to read.
//! \param [in] conversion conversion_matrix describing the yuv color space.
//! \returns A new xyuv::yuv_image with the new pixel data of \a rgbImage_in.
yuv_image rgb_to_yuv_image(const rgb_image &rgbImage_in, const xyuv::conversion_matrix &conversion);

//! \brief Scale a 444 yuv_image.
//!
//! \details Change the dimensions of a 444 yuv_image.
//! \pre \a yuva_444 is has subsampling 444.
//! \param [in] yuva_444, image to scale. Must be 444 sampled.
//! \param [in] new_width, target width.
//! \param [in] new_height, target height.
//! \returns A new scaled yuv_image.
yuv_image scale_yuv_image(const yuv_image &yuva_444, uint32_t new_width, uint32_t new_height);

//! \brief Crop a yuv_image.
//!
//! \details Crop away the edges of a 444 yuv_image.
//! \pre \a yuva_444 is has subsampling 444.
//! \param [in] yuva_444 input. Must be 444 sampled.
//! \param [in] left columns to crop from the left.
//! \param [in] top rows to crop from the top.
//! \param [in] right columns to crop from the right.
//! \param [in] bottom  rows to crop from the bottom.
yuv_image crop_yuv_image(
        const yuv_image &yuva_444,
        uint32_t left,
        uint32_t top,
        uint32_t right,
        uint32_t bottom
);


} // namespace xyuv