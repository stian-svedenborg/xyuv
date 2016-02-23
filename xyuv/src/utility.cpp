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
#include <cstring>

namespace xyuv {

    std::vector<std::string> list_files_in_folder(const std::string &dir_path) {
        DIR *dirp = opendir(dir_path.c_str());

        std::vector<std::string> ret_val;
        if (dirp) {
            struct dirent *current = NULL;
            struct dirent storage;


            readdir_r(dirp, &storage, &current);


            while (current != NULL) {
                if (current->d_type != DT_DIR) {
                    ret_val.push_back(std::string(current->d_name));
                }
                readdir_r(dirp, &storage, &current);
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
} // namespace xyuv
#endif

