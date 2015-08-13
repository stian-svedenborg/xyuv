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

#include <xyuv.h>
#include <xyuv/config_manager.h>
#include <iosfwd>
#include <vector>

struct options {
    // Paths to additional format files. Will try to load them as format first, then
    // chroma siting, then rgb_matrix.
    std::vector<std::string> additional_format_files;

    // Paths to additional config directories to load.
    std::vector<std::string> additional_config_directories;

    // files of raw data.
    // if no output file name has been specified the resulting filename is
    // the same as the input - suffix + ".xyuv"
    std::vector<std::string> input_files;

    // Strings to format templates to be used in the output.
    // Must be either exactly one or one per input file.
    std::vector<std::string> output_formats;

    // Strings to chroma sitings to be used in the output.
    // Must be either exactly one or one per input file.
    std::vector<std::string> output_siting;

    // Strings to conversion matrices to be used in the output.
    // Must be either exactly one or one per input file.
    std::vector<std::string> output_matrix;

    // Strings to output file names to be used in the output.
    // output_name.size() == 1 -> concatinate == true.
    // output_name.size() == 0 -> output_name[i] = input_files[i] - suffix + ".xyuv"
    // Otherwise the number of output names must exactly match the number of input names.
    std::vector<std::string> output_name;

    // Dimensions of the target image.
    uint32_t image_w = 0;
    uint32_t image_h = 0;

    // Write multiple frames to output.
    bool concatinate = false;

    // Write image to file
    bool writeout = true;

    // Display the resulting image.
    bool display = false;

    // List all loaded formats and quit.
    bool list_all_formats = false;

    // Print help and quit. (Superseeds list_all_formats).
    bool print_help = false;
};

class XYUVHeader {
public:
    XYUVHeader();

    void Run(const options & options);

    options ParseArgs(int argc, char ** argv);

protected:
    xyuv::frame AddHeader(const xyuv::format_template & fmt_template,
                   const xyuv::chroma_siting & siting,
                   const xyuv::conversion_matrix & matrix,
                   uint32_t image_w,
                   uint32_t image_h,
                   std::istream & input_stream
    );

    void Display(const xyuv::frame & frm);

    void PrintHelp();

    void PrintAllFormats();

private:
    xyuv::config_manager config_manager_;
};

