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
#include <climits>

namespace xyuv {

/** \file Wrappers for endianess support. */

#ifdef _MSC_VER
#   include <Winsock2.h>
#   define htobe64(x) htonll(x)
#   define htobe32(x) htonl(x)
#   define htobe16(x) htons(x)
#   define be64toh(x) ntohll(x)
#   define be32toh(x) ntohl(x)
#   define be16toh(x) ntohs(x)
#elif defined(__linux__)
#   include <endian.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#   include <sys/endian.h>
#elif defined(__OpenBSD__)
#   include <sys/types.h>
#   define be16toh(x) betoh16(x)
#   define be32toh(x) betoh32(x)
#   define be64toh(x) betoh64(x)
#elif defined(__APPLE__)
#	include <libkern/OSByteOrder.h>
#	define htobe16(x) OSSwapHostToBigInt16(x)
#	define htobe32(x) OSSwapHostToBigInt32(x)
#	define htobe64(x) OSSwapHostToBigInt64(x)
#	define be16toh(x) OSSwapBigToHostInt16(x)
#	define be32toh(x) OSSwapBigToHostInt32(x)
#	define be64toh(x) OSSwapBigToHostInt64(x)
#endif

inline uint64_t host_to_be(uint64_t val) {
	return htobe64(val);
}

inline uint32_t host_to_be(uint32_t val) {
	return htobe32(val);
}

inline uint16_t host_to_be(uint16_t val) {
	return htobe16(val);
}

inline uint8_t host_to_be(uint8_t val) {
	return val;
}

inline bool host_to_be(bool val) {
	return val;
}

inline uint64_t be_to_host(uint64_t val) {
	return be64toh(val);
}

inline uint32_t be_to_host(uint32_t val) {
	return be32toh(val);
}

inline uint16_t be_to_host(uint16_t val) {
	return be16toh(val);
}

inline uint8_t be_to_host(uint8_t val) {
	return val;
}

inline bool be_to_host(bool val) {
	return val;
}

} // namespace xyuv