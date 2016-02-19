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

#include <cstdint>

namespace xyuv {
    struct format;

// NB! PACKED_STRUCT is only valid within this file.
#ifdef _MSC_VER
    #    define PACKED_STRUCT struct
#    pragma pack( push, 1 )
#else
#    define PACKED_STRUCT struct __attribute__ ((packed, aligned(1)))
#endif

    extern const uint32_t CURRENT_FILE_FORMAT_VERSION;


    PACKED_STRUCT io_file_header {
        // File format specific
        char magic[8]; // Must be: "XYUV_FMT" (no '\0')
        uint32_t checksum; // MD5 Checksum of all following fields (header only).
        uint16_t version;  // Version of fileformat

        // Information needed to quickly read the binary data without having to parse remainder of header.
        uint16_t offset_to_data; // this + offset is the first byte of the workload.
        uint64_t payload_size;     // Size of this frame, this + offset_to_data + payload_size gives the address of the next frame header.
    };

#ifdef _MSC_VER
#    pragma pack( pop )
#endif

} // namespace xyuv