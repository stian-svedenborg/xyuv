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

#include "XYUVHeader.h"
#include <getopt.h>
#include <iostream>
#include <iomanip>
#include <unordered_map>

void XYUVHeader::PrintHelp() {
    std::cout <<
    "xyuv-header, version 0.1_beta\n"

    << std::endl;
}

static ::options::output_modes interpret_output_mode(const std::string &optarg) {
    std::unordered_map<std::string, ::options::output_modes> map {
            {"xyuv", ::options::output_modes::XYUV},
            {"magick", ::options::output_modes::IMAGEMAGICK},
    };

    auto it = map.find(optarg);
    if (it == map.end()) {
        it = map.begin();
        std::string error_message = "Invalid output mode: '" + optarg + "' must be one of { " + it->first;
        while (++it != map.end()) {
            error_message += ", " + it->first;
        }
        error_message += " }";
        throw std::invalid_argument(error_message);
    }
    return it->second;
}

::options XYUVHeader::ParseArgs(int argc, char ** argv) {
    struct option long_opts[] = {
            { "additional-format-dir", required_argument, 0, 'F'},
            { "output", required_argument, 0, 'o'},
            { "format-template", required_argument, 0, 'f'},
            { "chroma-siting", required_argument, 0, 's'},
            { "conversion-matrix", required_argument, 0, 'm'},
            { "width", required_argument, 0, 'w'},
            { "height", required_argument, 0, 'h'},
            { "output-mode", required_argument, 0, 'p'},
            { "cat", no_argument, 0, 'c'},
            { "display", no_argument, 0, 'd'},
            { "no-writeout", no_argument, 0, 'n'},
            { "list", no_argument, 0, 'l'},
            { "help", no_argument, 0, '?'},
            {}
    };
    int index = 0;
    int c = -1;
    const char * const shortopts = "?lndcF:o:f:h:w:m:s:";

    ::options options;
    while ( (c = getopt_long(argc, argv, shortopts, long_opts, &index )) != -1 ) {
        switch (c) {
            case 'F':
                options.additional_config_directories.push_back(optarg);
                break;
            case 'o':
                options.output_name.push_back(optarg);
                break;
            case 'f':
                options.output_formats.push_back(optarg);
                break;
            case 's':
                options.output_siting.push_back(optarg);
                break;
            case 'm':
                options.output_matrix.push_back(optarg);
                break;
            case 'w':
                options.image_w = static_cast<uint32_t>(strtoul(optarg, nullptr, 0));
                break;
            case 'h':
                options.image_h = static_cast<uint32_t>(strtoul(optarg, nullptr, 0));
                break;
            case 'c':
                options.concatinate = true;
                break;
            case 'd':
                options.display = true;
                break;
            case 'n':
                options.writeout = false;
                break;
            case 'l':
                options.list_all_formats = true;
                break;
            case '?':
                options.print_help = true;
                break;
            case 'p':
                options.output_mode = interpret_output_mode(optarg);
                break;
            default:
                throw std::invalid_argument("Unknown argument :'" + std::string(optarg) + "'");
        }
    }

    // If no arguments are given print help.
    if (argc == 1) {
        options.print_help = true;
    }

    while (optind < argc) {
        options.input_files.push_back(argv[optind++]);
    }

    return options;
}

void XYUVHeader::PrintAllFormats() {
    std::cout << "Available format templates:\n";
    std::cout << "  | Key    |  Subsampling |\n";
    for (const auto & fmt : config_manager_.get_format_templates()) {
        std::cout   << "    " << std::left << std::setw(10) << fmt.first
                    << "  <- " << static_cast<int>(fmt.second.subsampling.macro_px_w)
                    << 'x' << static_cast<int>(fmt.second.subsampling.macro_px_h) << '\n';
    }

    std::cout << "Available chroma sitings:\n";
    std::cout << "  | Key    |  Subsampling |\n";
    for (const auto & siting : config_manager_.get_chroma_sitings()) {
        std::cout   << "    " << std::left << std::setw(10) << siting.first
        << "  <- " << static_cast<int>(siting.second.subsampling.macro_px_w)
        << 'x' << static_cast<int>(siting.second.subsampling.macro_px_h) << '\n';
    }

    std::cout << "Available conversion matrices:\n";
    std::cout << "  | Key           |\n";
    for (const auto & conv : config_manager_.get_conversion_matrices()) {
        std::cout   << "    " << std::left << std::setw(10) << conv.first << '\n';
    }

    std::cout << std::endl;
}