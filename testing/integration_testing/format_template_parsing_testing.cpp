/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Stian Valentin Svedenborg
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

#include <gtest/gtest.h>
#include <xyuv/frame.h>
#include <xyuv/format_template.h>
#include <xyuv.h>
#include <xyuv/yuv_image.h>
#include "../../xyuv/src/config_parser.h"
#include "../TestResources.h"
#include "../../xyuv/src/assert.h"

using namespace xyuv;

//! In this test we want to stress the Toplevel API and ensure the results are sane.

TEST(FormatTemplate, LoadAllFormats) {

    std::string test_file = Resources::get().get_data_dir() + "formats/px_fmt/RGBA8888_DX_standard_swizzle";

    uint32_t w = 128;
    uint32_t h = 128;

    auto matrix = Resources::get().config().get_conversion_matrix("identity");

    for (auto & tmpl : Resources::get().get_all_formats())
    {
        std::string test_file = Resources::get().get_px_fmt_dir() + tmpl;

        format_template_old old_tmpl = Resources::get().config().get_format_template(tmpl);

        chroma_siting siting;
        siting.subsampling = old_tmpl.subsampling;
        siting.u_sample_point = {0,0};
        siting.v_sample_point = {0,0};

        format_template new_tmpl;
        new_tmpl.load_file(test_file);

        format old_format = create_format(w, h, old_tmpl, matrix, siting);
        format new_format = new_tmpl.inflate(w,h, siting, matrix);

        XYUV_ASSERT(old_format == new_format);

    }
}
