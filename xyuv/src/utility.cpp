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

} // namespace xyuv
#endif

