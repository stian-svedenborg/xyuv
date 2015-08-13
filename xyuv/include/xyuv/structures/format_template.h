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
#include "plane.h"
#include "subsampling.h"
#include "sample.h"

#include <array>
#include <vector>

namespace xyuv {

/** \brief A struct describing a class of pixel formats.
  *
  * \details
  * Together with a xyuv::chroma_siting  and a xyuv::conversion_matrix  it is used to instansiate a xyuv::format .
  *
  * If you have created a new pixel format you would integrate it into xyuv by defining a format_template.
  *
  * See documentation on LINK for information on how to integrate your own pixel formats with xyuv.
  * \todo Add link to format documentation
  */
struct format_template {
    //! \copydoc format::fourcc
    std::string fourcc;

    //! \copydoc format::subsampling
    xyuv::subsampling subsampling;

    //! \copydoc format::origin
    image_origin origin;

    //! \copydoc format::channel_blocks
    std::array<channel_block, 4> channel_blocks;

    //! \copydoc fromat::planes
    std::vector<plane_template> planes;
};

} // namespace xyuv