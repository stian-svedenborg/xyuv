/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Stian Valentin Svedenborg
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
#include <gtest/gtest.h>
#include <xyuv/config_manager.h>
#include <xyuv/frame.h>
#include <xyuv.h>
#include <fstream>

// Useful global constants
class Resources {
public:
    enum class TestImage {
        LENA,
        ODDS,
        TINY
    };

    const std::string get_data_dir() const {
        return "testing/integration_testing/test_data/";
    }

    const std::string get_file_basepath(TestImage image) const {
        switch (image) {
            case TestImage::LENA:
                return get_data_dir() + "lena512color";
            case TestImage::ODDS:
                return get_data_dir() + "odds";
            case TestImage::TINY:
                return get_data_dir() + "tiny";
            default:
                throw std::runtime_error("Unsupported value.");
        }
    }

    const std::string get_png_path(TestImage image) const {
        return get_file_basepath(image) + ".png";
    }

    xyuv::frame get_test_frame(TestImage image) {
        std::ifstream fin(get_file_basepath(image) + ".xyuv");
        xyuv::frame frame;
        xyuv::read_frame(&frame, fin);
        return frame;
    }

    const xyuv::config_manager & config() const;

    std::vector<std::string> get_all_formats() const;
    std::vector<std::string> get_all_conversion_matrices() const;

    static const Resources & get();
private:
    xyuv::config_manager config_;

    Resources();
};


