/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Stian Valentin Svedenborg
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

#include <xyuv/structures/color.h>
#include "TestResources.h"
#include "xyuv/color_conversion.h"
#include "../xyuv/src/config-parser/parse_error.h"

#include <gtest/gtest.h>

using namespace xyuv;

TEST(ConfigManager, LoadDefault) {
    xyuv::config_manager manager;
    EXPECT_NO_THROW(manager.load_configurations("formats"));
}

TEST(ConfigManager, LoadSingle) {
    xyuv::config_manager manager;
    EXPECT_NO_THROW(manager.load_format("testing/testdata/formats/ARGB8888"));
    EXPECT_NO_THROW(manager.get_format_template("ARGB8888"));

    EXPECT_NO_THROW(manager.load_format("testing/testdata/formats/410"));
    EXPECT_NO_THROW(manager.get_chroma_siting("410"));

    EXPECT_NO_THROW(manager.load_format("testing/testdata/formats/identity"));
    EXPECT_NO_THROW(manager.get_conversion_matrix("identity"));
}

TEST(ConfigManager, LoadSingleFailure) {
    xyuv::config_manager manager;
    EXPECT_THROW(manager.load_format("testing/testdata/invalid_formats/ARGB8888"), parse_error);
    EXPECT_THROW(manager.load_format("testing/testdata/invalid_formats/410"), parse_error);
    EXPECT_THROW(manager.load_format("testing/testdata/invalid_formats/identity"), parse_error);
}


