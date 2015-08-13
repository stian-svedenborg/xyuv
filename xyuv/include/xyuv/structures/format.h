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
#include "channel_block.h"
#include "chroma_siting.h"
#include "conversion_matrix.h"
#include "plane.h"
#include <vector>
#include <array>

namespace xyuv {

/** \brief A struct defining an instantiated format_template.
  * \details
  * Frame formats are handled in a special way in xyuv, it is important to understand that the distinction
  * between a xyuv::format  and a xyuv::format_template . A format_template is a generic template used
  * to describe the packing of data in a pixel format. A format_template is usually read from a JSON representation
  * and may be used to instantiate formats for images of different dimensions.
  *
  * A xyuv::format  on the other hand is the pixel format for one particular image, it has fields for important
  * constants like line stride and image dimensions. You may say that a format_template describes a class of images whilst
  * a format describe a single image.
  *
  * Users should avoid creating xyuv::format  directly and should instead use the xyuv::create_format()  function.
  */
struct format {

    //! \brief Fourcc identifier for this format. <a href="http://www.fourcc.org">fourcc.org</a>
    std::string fourcc;

    //! \brief The origin (location of pixel (0, 0) in this format.
    image_origin origin;

    //! \brief The width of the image in pixels.
    uint32_t image_w;

    //! \brief The height of the image in pixels.
    uint32_t image_h;

    //! \brief The size of the image data in bytes.
    //! \details Note that this is the size the image footprint will need. For formats with multiple planes this
    //! value will also include the necessary padding to ensure alignment of planes after the first.
    uint64_t size;

    //! \brief List of plane descriptors for the format.
    std::vector<plane> planes;

    //! \brief List of channel descriptors.
    //! \details The layout of the array is { Y, U, V, A }.
    std::array<channel_block, 4> channel_blocks;

    //! Copy of the chroma_siting used to instantiate this format.
    xyuv::chroma_siting chroma_siting;

    //! Copy of the conversion_matrix used to instantiate this format.
    xyuv::conversion_matrix conversion_matrix;
};

bool operator==(const format &lhs, const format &rhs);

} // namespace xyuv