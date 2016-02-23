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

#include <xyuv/frame.h>
#include <xyuv/structures/format.h>
#include "../assert.h"
#include "../config-parser/format_validator.h"
#include "versions/core_io_structs.h"
#include "versions/file_format_entry_point.h"
#include <limits>
#include <ostream>
#include <istream>

// Include the versions' entry points.
#include "versions/0/entry_point.h"
#include "versions/1/entry_point.h"
#include "endianess.h"



// Set the current format version
namespace xyuv { const uint32_t CURRENT_FILE_FORMAT_VERSION = 1; }


namespace xyuv {

// Helper functions:

uint64_t write_large_buffer(std::ostream & stream, const char * data, uint64_t size ) {
    XYUV_ASSERT(stream.good());
    uint64_t written = 0;
    while (written < size && stream) {
		// Write whatever is smaller of max(size_t) and size.
		std::streamsize to_write =
                static_cast<std::size_t>(std::min<uint64_t>(std::numeric_limits<decltype(to_write)>::max(), size));
		stream.write(data + written, sizeof(char)*to_write);
        written += to_write;
	}
    XYUV_ASSERT(stream.good());
    return written;
}

uint64_t  read_large_buffer(std::istream & stream, char * data, uint64_t size) {
    XYUV_ASSERT(stream.good());
    uint64_t read = 0;
    while (read < size && stream) {
        // Write whatever is smaller of max(size_t) and size.
        std::streamsize to_read =
                static_cast<std::streamsize >(std::min<uint64_t>(std::numeric_limits<decltype(to_read)>::max(), size));
        stream.read(data + read, sizeof(char)*to_read);
        read += to_read;
    }
    XYUV_ASSERT(stream.good());
    return read;
}

void write_frame(
        std::ostream & ostream,
        const xyuv::frame & frame,
        uint32_t version
) {
    std::vector<file_format_writer> file_format_writers {
            fileformat_version_0::write_header,
            fileformat_version_1::write_header
    };

    XYUV_ASSERT(version < file_format_writers.size() && "ERROR: File format too new for this library.");

    file_format_writers[version](ostream, frame.format);
    write_large_buffer(ostream, reinterpret_cast<const char *>(frame.data.get()), frame.format.size );
}

void write_frame(
        std::ostream & ostream,
        const xyuv::frame & frame
) {
    write_frame(ostream, frame, CURRENT_FILE_FORMAT_VERSION);
}

static void read_file_header(std::istream & istream, io_file_header& file_header) {
    // Read file header.
    istream.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    XYUV_ASSERT(istream);
}

void read_frame(
        xyuv::frame * frame,
        std::istream & istream
) {
    std::vector<file_format_loader> file_format_loaders {
            fileformat_version_0::read_header,
            fileformat_version_1::read_header
    };

    io_file_header file_header;
    read_file_header(istream, file_header);

    uint16_t version = be_to_host(file_header.version);
    XYUV_ASSERT(version < file_format_loaders.size() && "ERROR: File format too new for this library.");

    file_format_loaders[version](istream, frame->format, file_header);

    validate_format(frame->format);
    frame->data.reset( new uint8_t[frame->format.size]);
    read_large_buffer(istream, reinterpret_cast<char*>(frame->data.get()), frame->format.size);
}

} // namespace xyuv