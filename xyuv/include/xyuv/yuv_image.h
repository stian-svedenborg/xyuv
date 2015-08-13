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

#include <xyuv/structures/chroma_siting.h>
#include <xyuv/surface.h>
#include "xyuv/quantum.h"

namespace xyuv {

//! \brief Class representing a canonical yuv image.
//! \details A yuv_image is the canonical representation of a yuv image. That is, it is the intermediate representation
//! which is used when you convert from one format to another.
//!
//! In this canonical form, each plane is stored in separate surfaces (think of a surface as a two-dimensional array of
//! pixel quanti). Note however that not all images have all channels, any channel not present in the image will simply
//! be empty. e.g. If the image does not have an alpha channel, a_plane.empty() will return true.
//!
//! Most manipulation of a yuv_image should be done using the high-level functions declared in \link xyuv.h \endlink or
//! directly on the separate surfaces using the member functions of xyuv::surface.
//!
//! The origin of a yuv_image is defined to be the top left corner.
struct yuv_image {
    //! \brief Image width of the luma plane (y_plane).
    //! \details If the image has no luma plane, then image_w should still be the width of the luma plane (as though it was present)
    //! as dictated by the chroma siting.
    uint32_t image_w;

    //! \brief Image height of the luma plane (y_plane).
    //! \details If the image has no luma plane, then image_h should still be the height of the luma plane (as though it was present)
    //! as dictated by the chroma siting.
    uint32_t image_h;

    //! \brief Chroma siting, describing how to the image is subsampled.
    //! \note All yuv_images have a chroma siting, if the image is not subsampled or does not have chroma channels, then
    //! the chroma siting should simply equal that of subsampling 444. (i.e. no subsampling).
    xyuv::chroma_siting siting;

    //! \brief Surface for the luma channel.
    //! \details Canonical values in the in the y_plane are normalized to the range [0.0, 1.0].
    surface<pixel_quantum> y_plane;

    //! \brief Surface for the first chroma channel.
    //! \details Canonical values in the in the u_plane are normalized to the range [-0.5, 0.5].
    surface<pixel_quantum> u_plane;

    //! \brief Surface for the second chroma channel.
    //! \details Canonical values in the in the v_plane are normalized to the range [-0.5, 0.5].
    surface<pixel_quantum> v_plane;

    //! \brief Surface for the alpha channel.
    //! \details Canonical values in the in the a_plane are normalized to the range [0.0, 1.0].
    surface<pixel_quantum> a_plane;
};

//! \brief Create and initialize an empty yuv_image.
//! \details The newly created image will have all existing channels set to 0.0.
//! \param [in] image_w Width of the luma plane (i.e. the full resolution of the image) regardless of whether the image has a Y channel.
//! \param [in] image_h Height of the luma plane (i.e. the full resolution of the image) regardless of whether the image has a Y channel.
//! \param [in] siting Target chroma siting of the image.
//! \param [in] has_Y Boolean indicating whether the image should have a luma channel.
//! \param [in] has_U Boolean indicating whether the image should have the first chroma channel.
//! \param [in] has_V Boolean indicating whether the image should have the second chroma channel.
//! \param [in] has_A Boolean indicating whether the image should have a transparency channel.
yuv_image create_yuv_image(
        uint32_t image_w,
        uint32_t image_h,
        const xyuv::chroma_siting &siting,
        bool has_Y = true,
        bool has_U = true,
        bool has_V = true,
        bool has_A = true
);

//! \brief Create and initialize an empty full resolution yuv_image.
//! \details This function behaves exactly like \link create_yuv_image(uint32_t, uint32_t, const xyuv::chroma_siting &, bool, bool, bool, bool) \endlink
//!  except that the chroma siting will be 444. i.e. No subsampling.
//!
//! The newly created image will have all existing channels set to 0.0.
//! \param [in] image_w Width of the luma plane (i.e. the full resolution of the image) regardless of whether the image has a Y channel.
//! \param [in] image_h Height of the luma plane (i.e. the full resolution of the image) regardless of whether the image has a Y channel.
//! \param [in] has_Y Boolean indicating whether the image should have a luma channel.
//! \param [in] has_U Boolean indicating whether the image should have the first chroma channel.
//! \param [in] has_V Boolean indicating whether the image should have the second chroma channel.
//! \param [in] has_A Boolean indicating whether the image should have a transparency channel.

yuv_image create_yuv_image_444(
        uint32_t image_w,
        uint32_t image_h,
        bool has_Y = true,
        bool has_U = true,
        bool has_V = true,
        bool has_A = true
);

} // namespace xyuv