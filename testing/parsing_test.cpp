/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2017 Stian Valentin Svedenborg
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
#include "../xyuv/src/config-parser/minicalc/ast.h"
#include <vector>

const static std::vector<std::pair<std::string, AST::value>> expressions {
        {"7", AST::value::integer(7)},
        {"1+2",  AST::value::integer(3)},
        {"3-5",  AST::value::integer(-2)},
        {"2*3",  AST::value::integer(6)},
        {"12/3",  AST::value::integer(4)},
        {"3**3",  AST::value::integer(27)},
        {"(8)",  AST::value::integer(8)},
        {"-56",  AST::value::integer(-56)},
        {"2 + -43",  AST::value::integer(-41)},
        {"abs(-54)",  AST::value::integer(54)},

        {"gcd(2,1)",  AST::value::integer(1)},
        {"gcd(10,5)",  AST::value::integer(5)},
        {"gcd(9,15)",  AST::value::integer(3)},
        {"gcd(1,1)",  AST::value::integer(1)},
        {"gcd(13,7)",  AST::value::integer(1)},

        {"lcm(2,1)",  AST::value::integer(2)},
        {"lcm(10,5)",  AST::value::integer(10)},
        {"lcm(9,15)", AST::value::integer(3*3*5)},
        {"lcm(1,1)",  AST::value::integer(1)},
        {"lcm(13,7)", AST::value::integer(13*7)},

        // Boolean
        {"true",  AST::value::boolean(true)},
        {"false", AST::value::boolean(false)},

        {"!true",  AST::value::boolean(!true)},
        {"!false", AST::value::boolean(!false)},

        {"!(1==2)",  AST::value::boolean(true)},

        {"1==1", AST::value::boolean(true)},
        {"1==2", AST::value::boolean(false)},



        {"1 + 1 == 2", AST::value::boolean(true)},

        {"1 < 2", AST::value::boolean(true)},
        {"1 < 1", AST::value::boolean(false)},
        {"2 < 1", AST::value::boolean(false)},

        {"1 > 2", AST::value::boolean(false)},
        {"1 > 1", AST::value::boolean(false)},
        {"2 > 1", AST::value::boolean(true)},
        {"1 <= 2", AST::value::boolean(true)},
        {"1 <= 1", AST::value::boolean(true)},
        {"2 <= 1", AST::value::boolean(false)},
        {"1 >= 2", AST::value::boolean(false)},
        {"1 >= 1", AST::value::boolean(true)},
        {"2 >= 1", AST::value::boolean(true)},

        {"1 != 1", AST::value::boolean(false)},
        {"1!=2", AST::value::boolean(true)},

        {"if(true,  40 + 2, 20 + 4)",  AST::value::integer(42)},
        {"if(false, 40 + 2, 20 + 4)",  AST::value::integer(24)},

        {"true && true", AST::value::boolean(true)},
        {"true && false", AST::value::boolean(false)},
        {"false && true", AST::value::boolean(false)},
        {"false && false", AST::value::boolean(false)},

        {"true || true", AST::value::boolean(true)},
        {"true || false", AST::value::boolean(true)},
        {"false || true", AST::value::boolean(true)},
        {"false || false", AST::value::boolean(false)},
};

TEST(MiniCalc, Expressions) {

    for (auto expr : expressions) {
        SCOPED_TRACE(expr.first);
        MiniCalc expression(expr.first, nullptr);
        ASSERT_EQ(expr.second, expression.evaluate(nullptr));
    }
}