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
#include <sstream>
#include "../xyuv/src/paths.h"
#include "console_width.h"

constexpr int FALLBACK_COLUMN_WIDTH = 80;
constexpr int MAX_COLUMN_WIDTH = 150;
constexpr int INDENT_WIDTH = 8;

int get_adapted_console_width() {
    int console_width = get_console_width();
    if (console_width == 0) {
        console_width = FALLBACK_COLUMN_WIDTH;
    } else if (console_width > MAX_COLUMN_WIDTH) {
        console_width = MAX_COLUMN_WIDTH;
    }
    return console_width;
}

std::string format_string(uint32_t indent, uint32_t max_width, const std::string & str ) {
    std::size_t current_line_length = 0;
    std::string str_indent(indent, ' ');
    std::string result = str_indent;
    // Consume characters from input:
    std::string word;
    for (char c : str) {
        bool push_word = false;
        bool push_line = false;
        switch (c) {
            case '\n':
                push_line = true;
            case ' ':
                push_word = true;
                break;
            default:
                word += c;
                break;
        }

        if (push_word) {
            if (current_line_length + word.size() + 1 > max_width) {
                result += "\n" + str_indent;
                current_line_length = 0;
            }

            result += word + ' ';
            current_line_length += word.size() + 1;
            word.clear();

            if (push_line) {
                result += "\n" + str_indent;
                current_line_length = 0;
            }
        }
    }
    // Also push the last word:
    result += word;

    return result;
}

void print_help_section( const std::string &short_arg, const std::string & long_arg, const std::string & helpstring) {
    int console_width = get_adapted_console_width();

    console_width -= INDENT_WIDTH;

    std::cout << short_arg << std::endl << long_arg << std::endl;
    std::cout << format_string(INDENT_WIDTH, console_width, helpstring) << '\n' << std::endl;

}

void XYUVHeader::PrintHelp() {
    std::cout <<
    "xyuv-header, version 0.1_beta\n";
    print_help_section("-F PATH",
                       "--additional-format-dir PATH",
                       "Load an additional folder of formats. The utility will search for "
                       "format templates under <PATH>/" FORMATS_SEARCH_PATH ", chroma sitings under "
                       "<PATH>/" CHROMA_SITING_DIR " and conversion matrices under <PATH>/" CONVERSION_MATRICES_DIR ". "
                       "Any missing folder is ignored. Format files directly under <PATH> are also ignored."

    );

    print_help_section("-o OUTPUT_PATH",
                       "--output OUTPUT_PATH",
                       "Optionally override the output path for an input file, the first OUTPUT_PATH maps to the first INPUT_PATH, "
                       "the second OUTPUT_PATH to the second INPUT_PATH and so forth. If -o/--output is ommited the default OUTPUT_PATH "
                       "is the same as the INPUT_PATH but with suffix '.xyuv'."
                       "\nIf --cat is provided you must provide this argument exactly zero or one time. Otherwise the number of OUTPUT_PATHs "
                       "must exactly match the number of INPUT_PATHs or zero. "
                       "\nThe suffix of OUTPUT_PATH determines the operation performed on the encoded frame on writeout:"
                       "\n- xyuv           The raw data is written to an xyuv image."
                       "\n- bin, raw, yuv  The raw data directly, with no header."
                       "\n- *              If supported by imagemagick, the frame is "
                       "\n                 converted back to RGB and saved."
    );


    print_help_section("-c",
                       "--cat",
                       "Concatinate multiple frames into an .xyuv image. "
                               "\nNB! this tool currently does not support reading multi-frame .xyuv images."
    );

    print_help_section("-d",
                       "--display",
                       "Display the loaded image, (press ESC to close)."
                               "\nNB! This option may only be used for single input invocations."
    );

    print_help_section("-n",
                       "--no-writeout",
                       "Supress writeout (no files will be created or overwritten): This is useful for testing trial testing "
                               "new formats and for simple visualisation."
    );


    print_help_section("-l",
                       "--list",
                       "Print a list of all loaded formats, chroma sitings and conversion matrices and then quit."
    );

    print_help_section("-?",
                       "--help",
                       "Print this message and quit."
    );

    std::cout << format_string(0, get_adapted_console_width(),
                               "The following section provides parameters that affect how the images are loaded and stored. "
                         "Common for all modes is that the format supplied is the internal format used by the program to "
                         "store the image. This means that if you load a normal image format (e.g. PNG) and store it as Y16, "
                         "then all chroma information is lost. And even though you might save the image as a new PNG image the "
                         "resulting image will be greyscale. Furthermore, if you are loading a raw format(.bin, .raw, .yuv), "
                         "the format supplied should describe how to decode the supplied raw data. "
                         "Finally, if you supply an .xyuv input, then the input image will be converted to "
                         "the supplied format before it is stored internally, similarly to the behaviour for normal "
                         "image formats (such as PNG).\n")
            << std::endl;

    print_help_section("-f FMT_KEY",
                       "--format-template FMT_KEY",
                       "Set the load-format for an input file, this argument must be given once for each input file, or exactly once "
                               "meaning that one format will be used for all inputs."
                               "\nA list of valid keys is listed by the --list option."
    );

    print_help_section("-s CS_KEY",
                       "--chroma_siting CS_KEY",
                       "Set the chroma siting key for an input file, this argument must be given once for each input file, or exactly once "
                               "meaning that one siting will be used for all inputs."
                               "\nA list of valid keys is listed by the --list option."
    );

    print_help_section("-m CM_KEY",
                       "--conversion-matrix CM_KEY",
                       "Set the conversion matrix key for an input file, this argument must be given once for each input file, or exactly once "
                               "meaning that one matrix will be used for all inputs."
                               "\nA list of valid keys is listed by the --list option."
    );


    print_help_section("-w uint",
                       "--width uint",
                       "Set the width of the image. For raw input this is the size of the source, and for all inputs it "
                               "is the size of the output. This argument must be given once for each input file, or exactly once "
                               "meaning that one value will be used for all inputs."
    );

    print_help_section("-h uint",
                       "--height uint",
                       "Set the height of the image. For raw input this is the size of the source, and for all inputs it "
                               "is the size of the output. This argument must be given once for each input file, or exactly once "
                               "meaning that one value will be used for all inputs."
    );



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