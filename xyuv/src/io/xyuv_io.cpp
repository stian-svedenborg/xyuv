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
#include "io_structs.h"
#include "../config-parser/format_validator.h"
#include <limits>
#include <ostream>
#include <istream>

namespace xyuv {

const uint32_t CURRENT_FILE_FORMAT_VERSION = 0;

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

static void write_header(std::ostream & ostream, const xyuv::format & format ) {
    XYUV_ASSERT(ostream);

    // Write file header.
    io_file_header file_header;
    to_io_file_header(&file_header, format);
    ostream.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    XYUV_ASSERT(ostream);

    // Write frame header.
    io_frame_header frame_header;
    to_io_frame_header(&frame_header, format);
    ostream.write(reinterpret_cast<const char*>(&frame_header), sizeof(frame_header));
    XYUV_ASSERT(ostream);

    // Write each plane.
    for (auto & plane : format.planes) {
        io_plane_descriptor plane_descriptor;
        to_io_plane_descriptor(&plane_descriptor, plane);
        ostream.write(reinterpret_cast<const char*>(&plane_descriptor), sizeof(plane_descriptor));
        XYUV_ASSERT(ostream);
    }

    // Write each channel block.
    for (auto & channel_block : format.channel_blocks ) {
        io_channel_block block;
        to_io_channel_block(&block, channel_block);
        ostream.write(reinterpret_cast<const char*>(&block), sizeof(block));
        XYUV_ASSERT(ostream);
        // And for each block write all the samples.
        for (auto & sample : channel_block.samples ) {
            io_sample_descriptor sample_descriptor;
            to_io_sample_descriptor(&sample_descriptor, sample);
            ostream.write(reinterpret_cast<const char*>(&sample_descriptor), sizeof(sample_descriptor));
            XYUV_ASSERT(ostream);
        }
    }
}

void write_frame(
        std::ostream & ostream,
        const xyuv::frame & frame
) {
    write_header(ostream, frame.format);
    write_large_buffer(ostream, reinterpret_cast<const char *>(frame.data.get()), frame.format.size );
}

static void read_header(std::istream & istream, xyuv::format & format ) {
    XYUV_ASSERT(istream);

    // Read file header.
    io_file_header file_header;
    istream.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    XYUV_ASSERT(istream);

    uint16_t offset_to_data = 0; // unused
    uint32_t checksum = 0; // unused
    from_io_file_header(&format, &offset_to_data, &checksum, file_header);

    // Read frame header.
    io_frame_header frame_header;
    istream.read(reinterpret_cast<char*>(&frame_header), sizeof(frame_header));
    XYUV_ASSERT(istream);

    uint8_t n_planes = 0;
    from_io_frame_header(&format, &n_planes, frame_header);

    // Read each plane descriptor.
    for (uint32_t i = 0; i < n_planes; i++) {
        io_plane_descriptor plane_descriptor;
        xyuv::plane plane;
        istream.read(reinterpret_cast<char*>(&plane_descriptor), sizeof(plane_descriptor));
        XYUV_ASSERT(istream);

        from_io_plane_descriptor(&plane, plane_descriptor);

        format.planes.push_back(plane);
    }

    // Write each channel block.
    for (auto & channel_block : format.channel_blocks ) {
        io_channel_block block;
        uint32_t n_samples = 0;
        istream.read(reinterpret_cast<char*>(&block), sizeof(block));
        XYUV_ASSERT(istream);

        from_io_channel_block(&channel_block, &n_samples, block);

        // And for each block write all the samples.
        for (uint32_t i = 0; i < n_samples; i++ ) {
            io_sample_descriptor sample_descriptor;
            xyuv::sample sample;
            istream.read(reinterpret_cast<char*>(&sample_descriptor), sizeof(sample_descriptor));
            XYUV_ASSERT(istream);

            from_io_sample_descriptor(&sample, sample_descriptor);
            channel_block.samples.push_back(sample);
        }
    }
}

void read_frame(
        xyuv::frame * frame,
        std::istream & istream
) {
    read_header(istream, frame->format);
    validate_format(frame->format);
    frame->data.reset( new uint8_t[frame->format.size]);
    read_large_buffer(istream, reinterpret_cast<char*>(frame->data.get()), frame->format.size);

}

} // namespace xyuv