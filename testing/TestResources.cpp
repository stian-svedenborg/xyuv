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

//
// Created by stian on 30/05/15.
//

#include "../xyuv/src/paths.h"
#include "TestResources.h"

using namespace xyuv;

const config_manager & Resources::config() {
    return get().config_;
}

const Magick::Image & Resources::get_lena512() {
    return get().Lena512;
}
const Magick::Image & Resources::get_tiny() {
    return get().Tiny;
}
const Magick::Image & Resources::get_default() {
    return get_lena512();
}

const Resources & Resources::get() {
    static Resources instance;
    return instance;
}

std::vector<std::string> Resources::get_all_formats() {
    std::vector<std::string> ret_val;
    for (auto & elem : config().get_format_templates()) {
        ret_val.push_back(elem.first);
    }
    return ret_val;
}

Resources::Resources()
    : config_(FORMATS_SEARCH_PATH)
    , Lena512("testing/integration_testing/test_data/lena512color.png")
    , Tiny("testing/integration_testing/test_data/tiny.png")
{

}