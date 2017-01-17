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

#include <stdexcept>
#include "operations.h"

// Evaluator functions:

int minicalc_add(int lhs, int rhs) {
    return lhs + rhs;
}

int minicalc_sub(int lhs, int rhs) {
    return lhs - rhs;
}

int minicalc_mul(int lhs, int rhs) {
    return lhs * rhs;
}

int minicalc_div(int lhs, int rhs) {
    if (rhs == 0) {
        throw std::runtime_error("Divide by 0");
    }
    return lhs / rhs;
}

int minicalc_mod(int lhs, int rhs) {
    if (rhs == 0) {
        throw std::runtime_error("Divide by 0");
    }
    return lhs % rhs;
}

int minicalc_pow(int base, int exponent) {
    int result = 1;
    while (exponent > 0) {
        result *= base;
        --exponent;
    }
    return result;
}

int minicalc_negate(int v) {
    return -v;
}

int minicalc_next_multiple(int base, int multiplier) {
    int quotient_ceil = (base + (multiplier-1)) / multiplier;
    return quotient_ceil*multiplier;
}

int minicalc_abs(int v) {
    return v < 0 ? -v : v;
}

int minicalc_gcd(int lhs, int rhs) {
    if (lhs <= 0 || rhs <=0 ) {
        throw std::runtime_error("gcd() must have positive, non-zero operands");
    }
    if (lhs > rhs) {
        return minicalc_gcd(lhs-rhs, rhs);
    } else if (lhs < rhs) {
        return minicalc_gcd(lhs, rhs-lhs);
    }
    else {
        return lhs;
    }
}

int minicalc_lcm(int lhs, int rhs) {
    if (lhs <= 0 || rhs <=0 ) {
        throw std::runtime_error("lcm() must have positive, non-zero operands");
    }
    return lhs * rhs / minicalc_gcd(lhs, rhs);
}