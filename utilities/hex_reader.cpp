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
#include <iomanip>
#include "../xyuv/src/assert.h"

uint64_t hex2uint(const char * hex_str) {
    uint64_t v = 0;
    // Unrecognized characters are read as 0.
    while (*hex_str != '\0') {
        v <<= 4;
        if (*hex_str >= '0' && *hex_str <= '9') {
            v |= *hex_str - '0';
        }
        else if (*hex_str >= 'a' && *hex_str <= 'f') {
            v |= *hex_str - 'a' + 10;
        }
        else if (*hex_str >= 'A' && *hex_str <= 'F') {
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
            // Set : -> '\0' to make sure that only numbers show in the address.
            word.back() = '\0';
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
            // Set : -> '\0' to make sure that only numbers show in the address.
            word.back() = '\0';
            uint64_t addr = hex2uint(word.c_str());
            XYUV_ASSERT(starting_address < addr && "Unsorted hex files not supported.");
            current_offset = addr - starting_address;
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


void WriteHexFile(const std::string & filename, const uint8_t * data, uint64_t size) {
    constexpr uint32_t words_per_line = 4;
    const uint64_t uint32_rounds = size/sizeof(uint32_t);

    const uint32_t * word_ptr = reinterpret_cast<const uint32_t *>(data);

    std::ofstream fout(filename, std::ios::trunc | std::ios::out );

    if (!fout) {
        throw std::runtime_error("Could not open output file: '" + filename + "'");
    }

    fout.setf(std::ios::hex);

    for (uint64_t i = 0; i < uint32_rounds/words_per_line; i++) {
        XYUV_ASSERT(fout && "IO ERROR occured.");

        // Write address:
        uint64_t addr = i * sizeof(uint32_t)*words_per_line;
        fout << std::setw(16) << std::setfill('0') << std::noshowbase << std::hex << addr << ":";

        for (uint32_t i = 0; i < words_per_line; i++) {
            // Write lines of complete buffers.
            fout << ' ' <<  std::setw(8) << std::setfill('0') << std::noshowbase << std::hex << *word_ptr++;
        }
        fout << '\n';
    }


    uint64_t bytes_written = uint32_rounds * words_per_line * sizeof(uint32_t);
    if (bytes_written < size) {
        // Write last line.
        fout << std::setw(16) << std::setfill('0') << std::noshowbase << std::hex << bytes_written << ": ";

        // Write the last full words if possible
        while (bytes_written + sizeof(uint32_t) <= size) {
            fout << ' ' << std::setw(8) << std::setfill('0') << std::noshowbase << std::hex  << *word_ptr++;
            bytes_written += sizeof(uint32_t);
        }

        // If we are not done, write the last partial word.
        if (bytes_written < size) {
            uint32_t last_word = 0;
            const uint8_t * last_bytes = reinterpret_cast<const uint8_t *>(word_ptr);
            XYUV_ASSERT((size - bytes_written) < 4);
            for (uint32_t i = 0; bytes_written++ < size; i++) {
                last_word |= static_cast<uint32_t>(*(last_bytes++)) << (i*8);
            }
            fout << ' ' << std::setw(8) << std::setfill('0') << std::noshowbase << std::hex << last_word;
        }
    }

    XYUV_ASSERT(fout && "IO ERROR occured.");
}
