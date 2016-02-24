/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Stian Valentin Svedenborg
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

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>

uint64_t hex2uint(const char * hex_str) {
    uint64_t v = 0;
    while (*hex_str != '\0') {
        if (*hex_str >= '0' && *hex_str <= '9') {
            v <<= 4;
            v |= *hex_str - '0';
        }
        else if (*hex_str >= 'a' && *hex_str <= 'f') {
            v <<= 4;
            v |= *hex_str - 'a' + 10;
        }
        else if (*hex_str >= 'A' && *hex_str <= 'F') {
            v <<= 4;
            v |= *hex_str - 'A' + 10;
        }
        ++hex_str;
    }
    return v;
}

static inline void push_back_little_endian_bytes(std::vector<uint8_t> & buffer, const uint64_t value, const uint32_t n_bytes, const uint64_t offset) {
    if (offset > buffer.size()) {
        if (offset + n_bytes > buffer.capacity()) {
            buffer.reserve(buffer.capacity()*2);
        }
        buffer.resize(offset);
    }

    const uint8_t * bytes = reinterpret_cast<const uint8_t *>(&value);
    for (uint32_t i = 0; i < n_bytes; ++i) {
        buffer.push_back(bytes[i]);
    }
}


// Will load a hex file with lines on the format:
// (addr:)?([::hex_digit::]+[[::space::}*)+
std::vector<uint8_t> LoadHexFile(const std::string & filename) {
    uint64_t starting_address = 0ULL, current_offset = 0;

    std::ifstream fin(filename, std::ios::in );

    if (!fin) {
        throw std::runtime_error("Could not open input file: '" + filename + "'");
    }


    std::string line, word;


    std::vector<uint8_t> data;

    // Parse the first line specially as we need to set some state.
    if (std::getline(fin, line)) {
        // Set the current line
        std::istringstream str_in(line);

        // Read first word
        str_in >> word;

        // If the word ends with a ':' then this is a memory address:
        if (word.back() == ':') {
            starting_address = hex2uint(word.c_str());
        }
        else {
            // Otherwise this is a little endian data_value:
            push_back_little_endian_bytes(data, hex2uint(word.c_str()), static_cast<uint32_t>(word.size()/2), current_offset );
            current_offset += word.size()/2;
        }

        while (str_in >> word) {
            push_back_little_endian_bytes(data, hex2uint(word.c_str()), static_cast<uint32_t>(word.size()/2), current_offset );
            current_offset += word.size()/2;
        }
    }

    while (std::getline(fin, line)) {
        // Set the current line
        std::istringstream str_in(line);

        // Read first word
        str_in >> word;

        // If the word ends with a ':' then this is a memory address:
        if (word.back() == ':') {
            current_offset = hex2uint(word.c_str()) - starting_address;
        }
        else {
            // Otherwise this is a little endian data_value:
            push_back_little_endian_bytes(data, hex2uint(word.c_str()), static_cast<uint32_t>(word.size()/2), current_offset );
            current_offset += word.size()/2;
        }

        while (str_in >> word) {
            push_back_little_endian_bytes(data, hex2uint(word.c_str()), static_cast<uint32_t>(word.size()/2), current_offset );
            current_offset += word.size()/2;
        }
    }
    return std::move(data);
}

