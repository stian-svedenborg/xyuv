/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2021 Stian Valentin Svedenborg
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


#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <getopt.h>

#include <xyuv.h>
#include <xyuv/config_manager.h>
#include <xyuv/frame.h>
#include <sstream>
#include "helpers.h"
#include "../xyuv/src/utility.h"
#include "../xyuv/src/paths.h"

struct EncoderOptions {
    std::string input_file;
    std::string output_file;

    std::string format_template;
    std::vector<std::string> additional_format_template_locations;
    std::string chroma_siting;
    std::string conversion_matrix;

    bool dump_metadata = false;
    bool list_all_formats = false;
    bool split_planes = false;
};

class XYUVEncode {
public:
    void Run(EncoderOptions & options) {
        // Setup configuration manager
        xyuv::config_manager config_manager;
#ifdef INSTALL_FORMATS_PATH
        config_manager.load_configurations(INSTALL_FORMATS_PATH);
#endif
        // Add additional configuration paths:
        for (auto & path : options.additional_format_template_locations) {
            config_manager.load_configurations(path);
        }

        // If a list of all formats has been requested, print it and quit.
        if (options.list_all_formats) {
            Helpers::PrintAllFormats(config_manager);
            return;
        }

        // Load target format templates:
        xyuv::format_template target_fmt_template = config_manager.get_format_template(options.format_template);
        xyuv::chroma_siting target_chroma_siting = config_manager.get_chroma_siting(options.chroma_siting);
        xyuv::conversion_matrix target_conversion_matrix = xyuv::conversion_matrix::identity;
        if (options.conversion_matrix != "") {
            target_conversion_matrix = config_manager.get_conversion_matrix(options.conversion_matrix);
        }

        // Two modes are supported: Loading an xyuv-file and loading a rgb-image file.
        auto input_suffix = Helpers::ToLower(Helpers::GetSuffix(options.input_file));

        xyuv::frame frame;
        if (input_suffix == ".xyuv") {
            frame = Helpers::LoadXYUVFile(options.input_file);
            auto fmt = xyuv::create_format(frame.format.image_w, frame.format.image_h, target_fmt_template, target_conversion_matrix, target_chroma_siting);
            frame = xyuv::convert_frame(frame, fmt);
        }
        else {
            frame = Helpers::LoadConvertRGBImage(target_fmt_template, target_conversion_matrix, target_chroma_siting, options.input_file);
        }

        // Finally write the frame back.
        Helpers::WriteFrame(frame, options.output_file, options.split_planes);

        if (options.dump_metadata) {
            auto parts = Helpers::SplitSuffix(options.output_file);
            Helpers::WriteMetadata(frame, parts.first + "-meta");
        }

    }

    EncoderOptions ParseArgs(int argc, char * argv[]) {
        struct option long_opts[] = {
                {"additional-format-dir", required_argument, 0, 'F'},
                {"dump-metadata",         no_argument,       0, 'd'},
                {"split-planes",          no_argument,       0, 'z'},
                {"format-template",       required_argument, 0, 'f'},
                {"chroma-siting",         required_argument, 0, 's'},
                {"conversion-matrix",     required_argument, 0, 'm'},
                {"list",                  no_argument,       0, 'l'},
                {"help",                  no_argument,       0, '?'},
                {}
        };
        int index = 0;
        int c = -1;
        const char *const shortopts = "?ldF:o:f:m:s:";

        EncoderOptions options = {};
        while ((c = getopt_long(argc, argv, shortopts, long_opts, &index)) != -1) {
            switch (c) {
                case 'F':
                    options.additional_format_template_locations.push_back(optarg);
                    break;
                case 'f':
                    options.format_template = optarg;
                    break;
                case 's':
                    options.chroma_siting = optarg;
                    break;
                case 'm':
                    options.conversion_matrix = optarg;
                    break;
                case 'd':
                    options.dump_metadata = true;
                    break;
                case 'z':
                    options.split_planes = true;
                    break;
                case 'l':
                    options.list_all_formats = true;
                    break;
                case '?':
                    this->PrintHelp();
                    exit(0);
                    break;
                default:
                    throw std::invalid_argument("Unknown argument :'" + std::string(optarg) + "'");
            }
        }

        // If no arguments are given print help.
        if (argc == 1) {
            this->PrintHelp();
            exit(0);
        }

        // No need to continue if list all platforms is set:
        if (options.list_all_formats) {
            return options;
        }

        int remaining_args = argc - optind;
        if (remaining_args <= 0) {
            throw std::runtime_error("Missing required positional arguments: 'input_file' and 'output_file'.");
        } else if (remaining_args == 1) {
            throw std::runtime_error("Missing required positional arguments: 'output_file'.");
        }

        // The number of remaining arguments are at least 2:
        options.input_file = argv[optind++];
        options.output_file = argv[optind++];

        // Are there additional arguments?
        if (optind < argc) {
            std::ostringstream error_string("Unexpected positional arguments: [");
            while (optind < argc) {
                error_string << " " << argv[optind++] << ", ";
            }

            error_string << "]";
            throw std::runtime_error(error_string.str());
        }

        return options;
    }

    void PrintHelp() {
        std::cout << "xyuv-encode, version " XYUV_STRINGIFY(XYUV_VERSION) "\n";
        std::cout << Helpers::FormatString(0, Helpers::GetAdaptedConsoleWidth(),
                                           "Encode a single image to a .hex, .bin, .yuv or .xyuv file.\n"
                                                   "USAGE: xyuv-encode -f FMT_KEY -s CS_KEY -m CM_KEY [-F PATH]... [-d] [-l] [--split-planes] input_file output_file\n")
                  << std::endl;


        Helpers::PrintHelpSection("-f FMT_KEY",
                                  "--format-template FMT_KEY",
                                  "Set the pixel packing format for the output, this argument is required."
                                          "\nA list of valid format keys are listed by the --list option."
                                          "\nSpecial: You may supply a file-name directly to this flag, the tool will first try to "
                                          "\nopen a file with that name, if the file does not verify it will look for a corresponding "
                                          "\nkey in the store."
        );

        Helpers::PrintHelpSection("-s CS_KEY",
                                  "--chroma-siting CS_KEY",
                                  "Set the chroma siting key for the output, this argument is required."
                                          "\nA list of valid keys is listed by the --list option."
                                          "\nSpecial: You may supply a file-name directly to this flag, the tool will first try to "
                                          "\nopen a file with that name, if the file does not verify it will look for a corresponding "
                                          "\nkey in the store."
        );

        Helpers::PrintHelpSection("-m CM_KEY",
                                  "--conversion-matrix CM_KEY",
                                  "Set the color conversion matrix, if this argument is omitted the default will be \"identity\", i.e. no color transform."
                                          "\nA list of valid keys is listed by the --list option."
                                          "\nSpecial: You may supply a file-name directly to this flag, the tool will first try to "
                                          "\nopen a file with that name, if the file does not verify it will look for a corresponding "
                                          "\nkey in the store."
        );


        Helpers::PrintHelpSection("-F PATH",
                                  "--additional-format-dir PATH",
                                  "Load an additional folder of formats. The utility will search for "
                                          "format templates under <PATH>/" FORMATS_SEARCH_PATH ", chroma sitings under "
                                          "<PATH>/" CHROMA_SITING_DIR " and conversion matrices under <PATH>/" CONVERSION_MATRICES_DIR ". "
                                          "Any missing folder is ignored. Format files directly under <PATH>/ are also ignored."
        );

        Helpers::PrintHelpSection("",
                                  "input_path",
                                  "Path to image to load, can be "
                                #if defined(USE_IMAGEMAGICK) && USE_IMAGEMAGICK
                                  "any image file supported by ImageMagick or "
                                #elif defined(USE_LIBPNG) && USE_LIBPNG
                                  "a png image or "
                                #endif
                                  "an .xyuv image."
        );

        Helpers::PrintHelpSection("",
                                  "output_path",
                                  "Path to encoded image, the supported file suffixes are:"
                                          "\n- xyuv           The raw data is written to an xyuv image."
                                          "\n- bin, raw, yuv  The raw data directly, with no header. (See also --dump-metadata)"
                                          "\n- hex            The raw data converted to hex, with no header. (See also --dump-metadata)"
        );

        Helpers::PrintHelpSection("-d",
                                  "--dump-metadata",
                                  "When outputting to .raw, .bin, .yuv or .hex additionally output the metadata to a json file. This file will be named <basename>-meta.json"
        );

        Helpers::PrintHelpSection("",
                                  "--split-planes",
                                  "When outputting to .raw, .bin, .yuv or .hex, split each plane to a separate file. "
                                          "\nThese files will be named <basename>-p<X>.<suffix>. "
        );

        Helpers::PrintHelpSection("-l",
                                  "--list",
                                  "Print a list of all loaded formats, chroma sitings and conversion matrices and then quit."
        );

        Helpers::PrintHelpSection("-?",
                                  "--help",
                                  "Print this message and quit."
        );

    }
};

int main(int argc, char *argv[]) {
    try {
        auto prog = XYUVEncode();
        auto options = prog.ParseArgs(argc, argv);
        prog.Run(options);
    }
    catch (std::exception & e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

}
