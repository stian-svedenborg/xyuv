#pragma once

#include <stdexcept>

namespace xyuv {
    struct format;
    struct format_template_old;

    bool validate_format(const xyuv::format &format);

    // @brief Returns true if there are no gross violations by the format template.
    // @brief Retruns false if two or more samples are overlapping or a sample is overflowing a block.
    bool validate_format_template(const xyuv::format_template_old &format_template);

};
