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
#include <xyuv/frame.h>
#include <fstream>
#include <memory>
#include <unordered_set>

void XYUVHeader::Run(const ::options & options) {

    // If help has been requested, print it and quit.
    if (options.print_help) {
        PrintHelp();
        return;
    }
    // Otherwise do something useful.
    // First Load all additional formats.
    for (const auto & path : options.additional_config_directories) {
        config_manager_.load_configurations(path);
    }

    // If a list of all formats has been requested, print it and quit.
    if (options.list_all_formats) {
        PrintAllFormats();
        return;
    }

    // Try to aquire each input stream and output stream.
    std::vector<std::unique_ptr<std::istream>> istreams;
    for ( const auto & path : options.input_files ) {
        std::unique_ptr<std::istream> istream{new std::ifstream(path, std::ios::binary)};
        if (!(*istream)) {
            throw std::runtime_error("Could not open input file: '" + path + "'");
        }
        istreams.emplace_back(std::move(istream));
    }

    // Try to aquire each output stream.
    std::vector<std::unique_ptr<std::ostream>> ostreams;
    if (options.writeout) {
        // Make a set of output names to do argument validation.
        std::unordered_set<std::string> out_name_set;
        std::vector<std::string> output_names;

        if ( options.output_name.size() == 0 ) {
            // If concatinating, use first file name as output.
            if (options.concatinate) {
                std::string out_path = options.input_files[0].substr(0, options.input_files[0].rfind('.',
                                                                                                     std::string::npos));
                out_name_set.emplace(out_path);
                output_names.emplace_back(out_path);
            }
            else {
                out_name_set.insert(options.output_name.begin(), options.output_name.end());
                output_names.insert(output_names.begin(), options.output_name.begin(), options.output_name.end());
            }
        }
        // Check that no two output files overwrite one another.
        if (out_name_set.size() != output_names.size()) {
            throw std::invalid_argument("The same output name is given more than once. This is illegal for one invocation of the program.");
        }

        // Check that no input file is the same as an output file.
        // We don't account for relative paths etc here, but we won't care for now.
        for (const auto & in_path : options.input_files) {
            if (out_name_set.find(in_path) != out_name_set.end()) {
                throw std::invalid_argument("File '" + in_path + "' given as both an input and an output file, this is illegal.");
            }
        }

        // Check that the number of output files matches the number of input files
        // or one.
        if (output_names.size() != 1 && output_names.size() != options.input_files.size() ) {
            throw std::invalid_argument("If supplied, the number of output files must exactly match the number of input files or one (which implies concatinate).");
        }

        // Now, open the output files.
        for ( const auto & path : output_names ) {
            std::unique_ptr<std::ostream> ostream{ new std::ofstream(path, std::ios::binary) };
            if (!(*ostream)) {
                throw std::runtime_error("Could not open output file: '" + path + "'");
            }
            ostreams.emplace_back(std::move(ostream));
        }
    }

    // Check validity of formats:
    std::vector<xyuv::format_template> format_templates;
    std::vector<xyuv::chroma_siting> sitings;
    std::vector<xyuv::conversion_matrix> matrices;

    for (const auto & format_template : options.output_formats ) {
        try {
            format_templates.push_back(config_manager_.get_format_template(format_template));
        } catch (std::runtime_error & e) {
            throw std::invalid_argument(std::string(e.what()) + ". Please check the spelling of the argument.");
        }
    }
    if (format_templates.size() != 1 && format_templates.size() != options.input_files.size()) {
        throw std::invalid_argument("The number of format templates must exactly match the number of input files or one (which implies all input files have the same format).");
    }

    for (const auto & siting : options.output_siting ) {
        try {
            sitings.push_back(config_manager_.get_chroma_siting(siting));
        } catch (std::runtime_error & e) {
            throw std::invalid_argument(std::string(e.what()) + ". Please check the spelling of the argument.");
        }
    }
    if (sitings.size() != 1 && sitings.size() != options.input_files.size()) {
        throw std::invalid_argument("The number of chroma sitings must exactly match the number of input files or one (which implies all input files have the same chroma siting).");
    }

    for (const auto & matrix : options.output_matrix ) {
        try {
            matrices.push_back(config_manager_.get_conversion_matrix(matrix));
        } catch (std::runtime_error & e) {
            throw std::invalid_argument(std::string(e.what()) + ". Please check the spelling of the argument.");
        }
    }
    if (matrices.size() != 1 && matrices.size() != options.input_files.size()) {
        throw std::invalid_argument("The number of conversion matrices must exactly match the number of input files or one (which implies all input files have the same conversion matrix).");
    }

    // Do some extra checking for the chroma siting, which must match the subsampling mode of the format.
    if (sitings.size() == 1 && format_templates.size() > 1) {
        for (std::size_t i = 0; i < format_templates.size(); i++) {
            if (!(sitings[0].subsampling == format_templates[i].subsampling)) {
                throw std::invalid_argument("Sub-sampling mismatch, for format template #"
                                            + std::to_string(i) + "'" + options.output_formats[i] + "'"
                                            + " corresponding chroma siting expects subsampling mode: "
                                            + std::to_string(sitings[0].subsampling.macro_px_w) + "x" + std::to_string(sitings[0].subsampling.macro_px_h)
                                            + " got: "
                                            + std::to_string(format_templates[i].subsampling.macro_px_w) + "x" + std::to_string(format_templates[i].subsampling.macro_px_h)
                );
            }
        }
    }

    if (options.display && istreams.size() != 1 ) {
        throw std::invalid_argument("--display only supported for a single input.");
    }

    // At this point everything looks good :) Lets load some formats.
    for ( std::size_t i = 0; i < istreams.size(); i++) {
        xyuv::frame frame = AddHeader(
                format_templates.size() == 1 ? format_templates[0] : format_templates[i],
                sitings.size() == 1 ? sitings[0] : sitings[i],
                matrices.size() == 1 ? matrices[0] : matrices[i],
                options.image_w,
                options.image_h,
                *(istreams[i])
        );

        if (options.writeout) {
            xyuv::write_frame(
                    ostreams.size() == 1 ? *(ostreams[0]) : *(ostreams[i]),
                    frame
            );
        }

        if (options.display) {
            Display(frame);
        }
    }
}

