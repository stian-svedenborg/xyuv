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

#pragma once
#include "xyuv/structures/format_template.h"
#include "xyuv/structures/conversion_matrix.h"
#include "xyuv/structures/chroma_siting.h"

#include <string>
#include <set>
#include <map>
#include <stdexcept>

namespace xyuv {

//! \brief Class to ease loading of configurations.
//!
//! \details This class provides simple access to configurations such as chroma sitings, format templates and conversion
//!          matrices added through the library's configuration files.
//!
//! \details When loading configurations from a predefined location, the configuration manager will use the preprocessor
//!          variables PX_FMT_DIR, CRHOMA_SITING_DIR and CONVERSION_MATRICES_DIR as the relative paths to each of the
//!          configuration folders respectively.
//!
//! \code{.cpp}
//! int main() {
//!
//!     // Create a new configuration manager.
//!     config_manager configurations;
//!
//!     // Load all chroma sitings under /usr/share/lib/xyuv/<CHROMA_SITING_DIR>,
//!     // load all format templates under /usr/share/lib/xyuv/<PX_FMT_DIR>
//!     // and load all conversion matrices under /usr/share/lib/xyuv/<CONVERSION_MATRICES_DIR>,
//!     configurations.load_configurations("/usr/share/lib/xyuv/");
//!
//!     //! Print all format template names to std::cout.
//!     for (auto & fmt : configurations.get_format_templates() ) {
//!         std::cout << fmt.first << endl;
//!     }
//!
//!     return 0;
//! }
//! \endcode

class config_manager {
public:

    //! \brief Default constructor, constructs an empty configuration manager.
    config_manager() = default;

    //! \brief Convenience constructor: Load configuration from path \a format_search_root.
    //! \details Equivalent to calling
    //! \code{.cpp}
    //! config_manager manager;
    //! manager.load_configurations(format_search_root);
    //! \endcode
    config_manager(const std::string &format_search_root);

    //! \brief Load configurations from path.
    //! \details This will search path pointed to by \a format_search_root and load each configuration into the
    //! called object.
    //! \details It will search for chroma siting configurtaions under \a format_search_root /<CHROMA_SITING_DIR>,
    //! format templates under \a format_search_root /<PX_FMT_DIR>
    //! and conversion matrices under \a format_search_root/<CONVERSION_MATRICES_DIR>. See the source for the default
    //! values for each of these definitions.
    void load_configurations(std::string format_search_root);

    //! \brief Load a single format specification from a path.
    //! \details This will first try to load the format as a pixel format, then as a chroma siting and finally as a conversion matrix.
    void load_format(std::string path);

    //! \brief Add a configuration with the given key.
    //! \details This will manually add a new format_template/chroma_siting/conversion_matrix to the configuration
    //! manager.
    //! \note If there already exists a configuration item with the given \a key, the previous item will be overwritten.
    void add(const std::string &key, const format_template &fmt_template);

    //! \copydoc add(const std::string &, const format_template &)
    void add(const std::string &key, const chroma_siting &siting);
    //! \copydoc add(const std::string &, const format_template &)
    void add(const std::string &key, const conversion_matrix &matrix);

    //! \brief Get a format_template with the given key.
    //! \throw If the item is not found, a std::runtime_error is thrown.
    format_template get_format_template(const std::string &key) const;

    //! \brief Get a chroma_siting with the given key.
    //! \throw If the item is not found, a std::runtime_error is thrown.
    chroma_siting get_chroma_siting(const std::string &key) const;

    //! \brief Get a conversion_matrix with the given key.
    //! \throw If the item is not found, a std::runtime_error is thrown.
    conversion_matrix get_conversion_matrix(const std::string &key) const;

    //! \brief Return all chroma sitings corresponding to a given subsampling.
    const std::set<std::string> &get_chroma_sitings(const subsampling &sampling) const;

    //! \brief Return all loaded format templates.
    const std::map<std::string, format_template> &get_format_templates() const;

    //! \brief Return all loaded chroma sitings.
    const std::map<std::string, chroma_siting> &get_chroma_sitings() const;

    //! \brief Return all loaded conversion matrices.
    const std::map<std::string, conversion_matrix> &get_conversion_matrices() const;

    //! \brief Static helper accessor that will attempt to load a format template matrix descriptor from \a path.
    static format_template load_format_template(const std::string &path);
    //! \brief Static helper accessor that will attempt to load a chroma siting descriptor from \a path.
    static chroma_siting load_chroma_siting(const std::string &path);
    //! \brief Static helper accessor that will attempt to load a conversion matrix descriptor from \a path.
    static conversion_matrix load_conversion_matrix(const std::string &path);

private:
    std::map<std::string, format_template> format_templates_;

    void load_format_templates(const std::string &dir_path);

    void load_chroma_sitings(const std::string &dir_path);

    void load_conversion_matrices(const std::string &dir_path);

    std::map<subsampling, std::set<std::string>> sub_sampling_to_chroma_siting;
    std::map<std::string, chroma_siting> chroma_sitings_;
    std::map<std::string, conversion_matrix> conversion_matrices_;
};

} // namespace xyuv