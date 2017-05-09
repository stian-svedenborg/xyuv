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

//! \file Defines function wrappers around common operations.

// Integer Operations
extern int minicalc_add(int lhs, int rhs);
extern int minicalc_sub(int lhs, int rhs);
extern int minicalc_mul(int lhs, int rhs);
extern int minicalc_div(int lhs, int rhs);
extern int minicalc_mod(int lhs, int rhs);
extern int minicalc_pow(int base, int exponent);
extern int minicalc_gcd(int lhs, int rhs);
extern int minicalc_lcm(int lhs, int rhs);
extern int minicalc_negate(int v);

//! \brief returns the smallest value >= \a base that is divisible by \a multiplier.
extern int minicalc_next_multiple(int base, int multiplier);
extern int minicalc_abs(int v);

// Logical Operations
extern int minicalc_logic_eq(int lhs, int rhs);
extern int minicalc_logic_ne(int lhs, int rhs);
extern int minicalc_logic_lt(int lhs, int rhs);
extern int minicalc_logic_gt(int lhs, int rhs);
extern int minicalc_logic_le(int lhs, int rhs);
extern int minicalc_logic_ge(int lhs, int rhs);
extern int minicalc_logic_and(int lhs, int rhs);
extern int minicalc_logic_or(int lhs, int rhs);
extern int minicalc_logic_neg(int bool_expr);

