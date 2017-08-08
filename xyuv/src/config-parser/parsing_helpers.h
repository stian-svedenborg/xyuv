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

#include <rapidjson/document.h>
#include "parse_error.h"

#ifndef NDEBUG
#   include <iostream>
#   define DBG_MSG(msg) std::cerr << msg << std::endl
#else 
#   define DBG_MSG(msg)
#endif

#include <cstdint>
#include <stdexcept>

//! \brief Declare a variable that is required to be present in a yaml-block.
#define DECLARE_REQUIRED(root, field, type) \
    if (!(root).HasMember(# field)) { \
        throw parse_error("Missing required field '" # field "'"); \
    }\
    rapidjson::Value * field = &((root)[# field]);\
    if (!field->Is##type()) {\
        throw parse_error("Wrong type for field '" # field "' expected " # type);\
    }

//! \brief Declare a variable that is optional in a yaml-block.
#define DECLARE_OPTIONAL(root, field, type) \
    rapidjson::Value * field = nullptr; \
    if ((root).HasMember(# field)) { \
        field = &((root)[# field]);\
    }\
    if (field != nullptr && !field->Is##type()) {\
        throw parse_error("Wrong type for field '" # field "' expected " # type);\
    }

//! \brief Declare a variable that is required to be present in a yaml-block.
#define DECLARE_REQUIRED_MINICALC(root, field) \
    if (!(root).HasMember(# field)) {\
        throw parse_error("Missing required field '" # field "'"); \
    } \
    rapidjson::Value *field = &((root)[# field]);


//! \brief Declare a variable that is optional in a yaml-block.
#define DECLARE_OPTIONAL_MINICALC(root, field) \
    rapidjson::Value * field = nullptr; \
    if ((root).HasMember(# field)) { \
        field = &((root)[# field]);\
    }

#define VALIDATE_VALUE_RANGE(min, max, value) do { \
    if ((value) < (min) || (max) < (value)) { \
        throw parse_error("Value out of range'" \
            "' expected value in [" + to_string(min) + ", " + to_string(max) + "] was " \
            + to_string(value));\
    } \
} while (0)

#define VALIDATE_RANGE(min, max, field, type) do { \
    if (field->Get##type() < (min) || (max) < field->Get##type()) { \
        throw parse_error("Value out of range for field '" # field \
            "' expected value in [" + to_string(min) + ", " + to_string(max) + "] was " \
            + to_string(field->Get##type()));\
    } \
} while (0)

namespace xyuv {

//! \brief Returns the line number in \a json for the character at \a offset.
uint32_t line_number_from_offset(const std::string &json, std::size_t offset);

} // namespace xyuv