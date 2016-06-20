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

#include <xyuv/config_manager.h>
#include "../assert.h"
#include "../to_string.h"
#include "../config_parser.h"
#include "../utility.h"
#include "../paths.h"
#include "parse_error.h"
#include "format_validator.h"
#include <iostream>
#include <stdexcept>

namespace xyuv {

// Helper functions

format_template config_manager::load_format_template(const std::string &path) throw(std::runtime_error, std::logic_error)
{
    xyuv::format_template format_template = parse_format_template(read_json(path));
    if (validate_format_template(format_template)) {
        return format_template;
    }
    else {
        throw std::runtime_error("Format template " + path + " did not validate.");
    }
}

chroma_siting config_manager::load_chroma_siting(const std::string &path) throw(std::runtime_error, std::logic_error)
{
    return parse_chroma_siting(read_json(path));
}

conversion_matrix config_manager::load_conversion_matrix(const std::string &path) throw(std::runtime_error, std::logic_error)
{
    return parse_conversion_matrix(read_json(path));
}



void config_manager::load_format_templates(const std::string &dir_path) {
    std::vector<std::string> files = list_files_in_folder(dir_path);

    for (auto &file : files) {
        try {
            xyuv::format_template format_template = load_format_template(dir_path + "/" + file);
            add(file, format_template);
        } catch (parse_error &e) {
            std::cerr << "Parse error in '" << dir_path << file << "': " << e.what() << std::endl;
        } catch (std::logic_error & e) {
            std::cerr << "Invalid format template '" << dir_path << file << "': " << e.what() << std::endl;
        }
    }
}

bool operator<(const subsampling &lhs, const subsampling &rhs);

void config_manager::load_chroma_sitings(const std::string &dir_path) {
    std::vector<std::string> files = list_files_in_folder(dir_path);

    for (auto &file : files) {
        try {
            xyuv::chroma_siting chroma_siting = load_chroma_siting(dir_path + "/" + file);
            add(file, chroma_siting);
        } catch (parse_error &e) {
            std::cerr << "Parse error in '" << dir_path << file << "': " << e.what() << std::endl;
        }
    }
}

void config_manager::load_conversion_matrices(const std::string &dir_path) {
    std::vector<std::string> files = list_files_in_folder(dir_path);

    for (auto &file : files) {
        try {
            xyuv::conversion_matrix conversion_matrix = load_conversion_matrix(dir_path + "/" + file);
            add(file, conversion_matrix);
        } catch (parse_error &e) {
            std::cerr << "Parse error in '" << dir_path << file << "': " << e.what() << std::endl;
        }
    }
}

config_manager::config_manager(const std::string &format_search_root) {
    load_configurations(format_search_root);
}

void config_manager::load_configurations(std::string format_search_root) {
    if(!(format_search_root.back() == '/' || format_search_root.back() == '\\')) {
        format_search_root += '/';
    }

    load_format_templates(format_search_root + PX_FMT_DIR);
    load_chroma_sitings(format_search_root + CHROMA_SITING_DIR);
    load_conversion_matrices(format_search_root + CONVERSION_MATRICES_DIR);
}

void config_manager::add(const std::string &key, const format_template &fmt_template) {
    format_templates_.emplace(key, fmt_template);
}

void config_manager::add(const std::string &key, const chroma_siting &siting) {
    chroma_sitings_.emplace(key, siting);
    sub_sampling_to_chroma_siting[siting.subsampling].emplace(key);
}

void config_manager::add(const std::string &key, const conversion_matrix &matrix) {
    conversion_matrices_.emplace(key, matrix);
}

const std::set<std::string> &config_manager::get_chroma_sitings(const subsampling &sampling) const {
    auto it = sub_sampling_to_chroma_siting.find(sampling);
    if (it == sub_sampling_to_chroma_siting.end()) {
        throw std::runtime_error("There are no chroma siting registered for subsampling ("
                                 + to_string(sampling.macro_px_w)
                                 + ", "
                                 + to_string(sampling.macro_px_h)
                                 + ")."
        );

    }
    return it->second;
}

xyuv::format_template     config_manager::get_format_template(const std::string &key) const throw(std::runtime_error) {
    auto it = format_templates_.find(key);
    if (it == format_templates_.end()) {
        throw std::runtime_error("There is no format template named " + key);
    }
    return it->second;
}

chroma_siting        config_manager::get_chroma_siting(const std::string &key) const throw(std::runtime_error) {
    auto it = chroma_sitings_.find(key);
    if (it == chroma_sitings_.end()) {
        throw std::runtime_error("There is no chroma siting named " + key);
    }
    return it->second;
}

conversion_matrix   config_manager::get_conversion_matrix(const std::string &key) const throw(std::runtime_error) {
    auto it = conversion_matrices_.find(key);
    if (it == conversion_matrices_.end()) {
        throw std::runtime_error("There is no conversion matrix named " + key);
    }
    return it->second;
}

const std::map<std::string, format_template> &config_manager::get_format_templates() const {
    return format_templates_;
}

const std::map<std::string, chroma_siting> &config_manager::get_chroma_sitings() const {
    return chroma_sitings_;
}

const std::map<std::string, conversion_matrix> &config_manager::get_conversion_matrices() const {
    return conversion_matrices_;
}

bool operator<(const subsampling &lhs, const subsampling &rhs) {
    return (lhs.macro_px_w << 16) + lhs.macro_px_h < (rhs.macro_px_w << 16) + rhs.macro_px_h;
}

} // namespace xyuv