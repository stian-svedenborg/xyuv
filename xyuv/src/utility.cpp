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

#include "utility.h"
#include "assert.h"


#ifdef _MSVC
#include <windows.h>
#include <tchar.h>
#include <memory>

namespace xyuv {

std::vector<std::string> list_files_in_folder(const std::string &dir_path) {
	HANDLE hFind;
	WIN32_FIND_DATA data;
	std::unique_ptr<TCHAR[]> wbuffer = nullptr;
	std::vector<std::string> files;

	std::string path = dir_path;
	if (dir_path.back() != '/') {
		path += '/';
	}

	int required_characters = MultiByteToWideChar(CP_ACP, 0, (path + '*').c_str(), -1, NULL, 0);
	wbuffer = std::make_unique<TCHAR[]>(required_characters);
	
	MultiByteToWideChar(CP_ACP, 0, (path + '*').c_str(), -1, wbuffer.get(), required_characters);
	CHAR tmp[512];
	hFind = FindFirstFile(wbuffer.get(), &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (lstrcmp(data.cFileName, TEXT(".")) == 0 || 
				lstrcmp(data.cFileName, TEXT("..")) == 0) {
				continue;
			}
			int strlen = WideCharToMultiByte(CP_ACP, 0, data.cFileName, -1, tmp, 512, NULL, FALSE);
			files.push_back(std::string(tmp, strlen));
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	
	return files;
}

} // namespace xyuv
#else

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

namespace xyuv {

    std::vector<std::string> list_files_in_folder(const std::string &dir_path) {
        DIR *dirp = opendir(dir_path.c_str());

        std::vector<std::string> ret_val;
        if (dirp) {
            struct dirent *current = readdir(dirp);

            while (current != NULL) {
                std::string d_name = current->d_name;

                // Dtype is not supported on all systems. So in the case it is not (or in the case of a symlink)
                // fall back to stat.
                if (current->d_type == DT_UNKNOWN || current->d_type == DT_LNK) {
                    struct stat entry;
                    std::string full_path = (dir_path + "/" + d_name);
                    // Stat will follow symlinks.
                    int err = stat(full_path.c_str(), &entry);
                    if (err) {
                        throw std::runtime_error("Could not stat: '" + full_path + "'");
                    }

                    if (S_ISREG(entry.st_mode)) {
                        ret_val.push_back(d_name);
                    }
                }
                // If it's supported, we rely on d_type (which is faster).
                else if (current->d_type == DT_REG) {
                    ret_val.push_back(d_name);
                }
                current = readdir(dirp);
            }

            closedir(dirp);
        }

        return ret_val;
    }

    uint32_t gcd(uint32_t u, uint32_t v)
    {
        // Copy-pasted from: https://en.wikipedia.org/wiki/Binary_GCD_algorithm
        int shift;

        /* GCD(0,v) == v; GCD(u,0) == u, GCD(0,0) == 0 */
        if (u == 0) return v;
        if (v == 0) return u;

        /* Let shift := lg K, where K is the greatest power of 2
              dividing both u and v. */
        for (shift = 0; ((u | v) & 1) == 0; ++shift) {
            u >>= 1;
            v >>= 1;
        }

        while ((u & 1) == 0)
            u >>= 1;

        /* From here on, u is always odd. */
        do {
            /* remove all factors of 2 in v -- they are not common */
            /*   note: v is not zero, so while will terminate */
            while ((v & 1) == 0)  /* Loop X */
                v >>= 1;

            /* Now u and v are both odd. Swap if necessary so u <= v,
               then set v = v - u (which is even). For bignums, the
               swapping is just pointer movement, and the subtraction
               can be done in-place. */
            if (u > v) {
                unsigned int t = v; v = u; u = t;}  // Swap u and v.
            v = v - u;                       // Here v >= u.
        } while (v != 0);

        /* restore common factors of 2 */
        return u << shift;
    }

    uint32_t lcm(uint32_t a, uint32_t b) {
        uint32_t prod = a*b;
        if (prod == 0) return 0;
        return (prod)/gcd(a, b);
    }

    uint32_t next_multiple(uint32_t base, uint32_t multiplier) {
        uint32_t quotient_ceil = (base + (multiplier-1)) / multiplier;
        return quotient_ceil*multiplier;

    }

    void poison_buffer(void *buffer, uint64_t buffersize) {
        const uint32_t poison_value = 0xDEADBEEF;

        uint64_t i = 0;
        for (; i + sizeof(poison_value) <= buffersize; i+= sizeof(poison_value)) {
            memcpy(static_cast<uint8_t *>(buffer) + i, &poison_value, sizeof(poison_value));
        }

        uint64_t remaining = buffersize - i;
        XYUV_ASSERT(remaining < sizeof(poison_value));
        if (remaining) {
            memcpy(static_cast<uint8_t *>(buffer) + i, &poison_value, remaining);
        }
    }
} // namespace xyuv
#endif

