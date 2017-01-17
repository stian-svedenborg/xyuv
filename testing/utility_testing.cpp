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

#include <gtest/gtest.h>
#include "../xyuv/src/utility.h"

TEST(Utility, ListFilesInFolder) {
	ASSERT_GT(xyuv::list_files_in_folder("formats/px_fmt/").size(), 0u);
}

TEST(Utility, PoisonBuffer) {

	// Test Aligned buffer.
	uint32_t buf0[4] = {};
	xyuv::poison_buffer(buf0+1, sizeof(uint32_t)*2);
	ASSERT_EQ(buf0[0], 0x0); // Check for underflow
	ASSERT_EQ(buf0[1], 0xDEADBEEF); // Check that order is correct for little endian deadbeef.
	ASSERT_EQ(buf0[2], 0xDEADBEEF);
	ASSERT_EQ(buf0[3], 0x0); // Check for overflow.

	// Test unaligned buffer
	uint8_t buf1[12] = {};
	xyuv::poison_buffer(buf1, sizeof(uint8_t)*10);
	ASSERT_EQ(buf1[0], 0xEF);
	ASSERT_EQ(buf1[1], 0xBE);
	ASSERT_EQ(buf1[2], 0xAD);
	ASSERT_EQ(buf1[3], 0xDE);
	ASSERT_EQ(buf1[4], 0xEF);
	ASSERT_EQ(buf1[5], 0xBE);
	ASSERT_EQ(buf1[6], 0xAD);
	ASSERT_EQ(buf1[7], 0xDE);
	ASSERT_EQ(buf1[8], 0xEF);
	ASSERT_EQ(buf1[9], 0xBE);
	ASSERT_EQ(buf1[10], 0x0);
	ASSERT_EQ(buf1[11], 0x0);

	memset(buf1, 0x0, 12*sizeof(uint8_t));

	xyuv::poison_buffer(buf1+2, sizeof(uint8_t)*10);
	ASSERT_EQ(buf1[0], 0x0);
	ASSERT_EQ(buf1[1], 0x0);
	ASSERT_EQ(buf1[2], 0xEF);
	ASSERT_EQ(buf1[3], 0xBE);
	ASSERT_EQ(buf1[4], 0xAD);
	ASSERT_EQ(buf1[5], 0xDE);
	ASSERT_EQ(buf1[6], 0xEF);
	ASSERT_EQ(buf1[7], 0xBE);
	ASSERT_EQ(buf1[8], 0xAD);
	ASSERT_EQ(buf1[9], 0xDE);
	ASSERT_EQ(buf1[10], 0xEF);
	ASSERT_EQ(buf1[11], 0xBE);

	memset(buf1, 0x0, 12*sizeof(uint8_t));

	xyuv::poison_buffer(buf1+2, sizeof(uint8_t)*9);
	ASSERT_EQ(buf1[0], 0x0);
	ASSERT_EQ(buf1[1], 0x0);
	ASSERT_EQ(buf1[2], 0xEF);
	ASSERT_EQ(buf1[3], 0xBE);
	ASSERT_EQ(buf1[4], 0xAD);
	ASSERT_EQ(buf1[5], 0xDE);
	ASSERT_EQ(buf1[6], 0xEF);
	ASSERT_EQ(buf1[7], 0xBE);
	ASSERT_EQ(buf1[8], 0xAD);
	ASSERT_EQ(buf1[9], 0xDE);
	ASSERT_EQ(buf1[10], 0xEF);
	ASSERT_EQ(buf1[11], 0x0);

	// Small buffer:
	memset(buf1, 0x0, 12*sizeof(uint8_t));
	xyuv::poison_buffer(buf1, sizeof(uint8_t)*3);
	ASSERT_EQ(buf1[0], 0xEF);
	ASSERT_EQ(buf1[1], 0xBE);
	ASSERT_EQ(buf1[2], 0xAD);
	ASSERT_EQ(buf1[3], 0x0);

	// Empty buffer:
	memset(buf1, 0x0, 12*sizeof(uint8_t));
	xyuv::poison_buffer(buf1, 0);
	ASSERT_EQ(buf1[0], 0x0);
	ASSERT_EQ(buf1[1], 0x0);
	ASSERT_EQ(buf1[2], 0x0);
	ASSERT_EQ(buf1[3], 0x0);
	ASSERT_EQ(buf1[4], 0x0);
	ASSERT_EQ(buf1[5], 0x0);
	ASSERT_EQ(buf1[6], 0x0);
	ASSERT_EQ(buf1[7], 0x0);
	ASSERT_EQ(buf1[8], 0x0);
	ASSERT_EQ(buf1[9], 0x0);
	ASSERT_EQ(buf1[10], 0x0);
	ASSERT_EQ(buf1[11], 0x0);

}