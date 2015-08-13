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
#include <iosfwd>
#include <cstdint>

namespace xyuv {
//! \brief Write a (potentially) large buffer to stream.
//! \details This is intended to workaround the possibility of std::size_t being too
//! small to hold the size of our images.
//! \warning I've never tested if this actually works for very large buffers, (or is even a problem).
//!          At best I presume it's implementation
//!          defined, so take care when using very very large images,
//!          i.e. images with size > std::numeric_limits<ssize_t>::max().
//! \param [in/out] stream input stream to read from.
//! \param [in] data Buffer to read data from, must be >= \a size bytes large.
//! \param [in] size Size of the data buffer.
//! \returns The number of bytes actually written.
uint64_t write_large_buffer(std::ostream &stream, const char *data, uint64_t size);

//! \brief Read a (potentially) large buffer to memory.
//! \details This is intended to workaround the possibility of std::size_t being too
//! small to hold the size of our images.
//! \warning I've never tested if this actually works for very large buffers, (or is even a problem).
//!          At best I presume it's implementation
//!          defined, so take care when using very very large images,
//!          i.e. images with size > std::numeric_limits<ssize_t>::max().
//! \param [in/out] stream input stream to read from.
//! \param [out] data Buffer where data will be written, must be >= \a size bytes large.
//! \param [in] size Size of the data buffer.
//! \returns The number of bytes actually read.
uint64_t read_large_buffer(std::istream &stream, char *data, uint64_t size);

};