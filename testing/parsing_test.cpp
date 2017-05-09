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

#include "../src/config-parser/minicalc/minicalc.h"
#include <vector>

const static std::vector<std::pair<std::string, uint64_t>> expressions {
        {"7", 7},
        {"1+2", 3},
        {"3-5", -2},
        {"2*3", 6},
        {"12/3", 4},
        {"3**3", 27 },
        {"(8)", 8},
        {"-56", -56},
        {"2 + -43", -41},
        {"abs(-54)", 54 },

        {"gcd(2,1)", 1},
        {"gcd(10,5)", 5},
        {"gcd(9,15)", 3},
        {"gcd(1,1)", 1},
        {"gcd(13,7)", 1},

        {"lcm(2,1)", 2},
        {"lcm(10,5)", 10},
        {"lcm(9,15)", 15*3},
        {"lcm(1,1)", 1},
        {"lcm(13,7)", 13*7},

        // Boolean
        {"int(true)",  uint64_t(true)},
        {"int(false)", uint64_t(false)},

        {"int(!true)",  uint64_t(!true)},
        {"int(!false)", uint64_t(!false)},

        {"int(!(1==2))",  1},
        {"int(! 1 == 2)", 1},

        {"int(1==1)", 1},
        {"int(1==2)", 0},

        {"int(1!=1)", 0},
        {"int(1!=2)", 1},

        {"int(1 + 1 == 2)", 1},

        {"int(1 < 2)", 1},
        {"int(1 < 1)", 0},
        {"int(2 < 1)", 0},

        {"int(1 > 2)", 0},
        {"int(1 > 1)", 0},
        {"int(2 > 1)", 1},

        {"int(1 <= 2)", 1},
        {"int(1 <= 1)", 1},
        {"int(2 <= 1)", 0},

        {"int(1 >= 2)", 0},
        {"int(1 >= 1)", 1},
        {"int(2 >= 1)", 1},

        {"if(true,  40 + 2, 20 + 4)", 42},
        {"if(false, 40 + 2, 20 + 4)", 24},

        {"int(true && true)", 1},
        {"int(true && false)", 0},
        {"int(false && true)", 0},
        {"int(false && false)", 0},

        {"int(true || true)", 1},
        {"int(true || false)", 1},
        {"int(false || true)", 1},
        {"int(false || false)", 0},
};

TEST(MiniCalc, Expressions) {

    for (auto expr : expressions) {
        SCOPED_TRACE(expr.first);
        MiniCalc expression(expr.first);
        ASSERT_EQ(expr.second, expression.evaluate(nullptr));
    }
}