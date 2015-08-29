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


#include "console_width.h"

// Acknowledgement: These functions are more or less copy paste from
// stack-overflow answers:
// http://stackoverflow.com/questions/8627327/how-to-get-number-of-characters-in-line-in-console-that-my-process-is-bind-to
// http://stackoverflow.com/questions/6657048/how-can-i-find-the-number-of-terminal-columns-from-a-c-c-program

#ifndef _MSVC

#include <sys/ioctl.h>

int get_console_width()
{
    struct winsize w;
    if (ioctl(0, TIOCGWINSZ, &w)) {
        return 0;
    }

    return w.ws_col;
}

#else

#include <windows.h>

int get_console_width()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if(!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return 0; // If not possible to get, return null and let application
        // deal with it.
    }
    else {
        return csbi.srWindow.Right-csbi.srWindow.Left;
    }
}

#endif