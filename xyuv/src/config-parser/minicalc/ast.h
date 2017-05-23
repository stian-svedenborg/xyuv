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

#include <functional>
#include <string>
#include <memory>
#include <ostream>
#include <deque>

class MiniCalc;

//! \file File contains abstract syntax tree definitions and helper functions.


namespace AST {
    enum class value_type {
        INT,
        BOOL,
        STRING,
        UNBOUND
    };
    std::string to_string(const AST::value_type & type);

    struct value {
        value_type type;
        using ival_t = int;
        using bval_t = bool;
        using sval_t = std::string;
        union {
            bval_t bval;
            ival_t ival;
        };
        sval_t sval;

	value() : type(value_type::UNBOUND) {}
	value(const value&);
    value & operator=(const value& rhs);

        static value boolean(bval_t bval) {
            value val;
            val.type = value_type::BOOL;
            val.bval = bval;
            return val;
        }

        static value integer(ival_t ival) {
            value val;
            val.type = value_type::INT;
            val.ival = ival;
            return val;
        }

        static value string(sval_t sval) {
            value val;
            val.type = value_type::STRING;
            val.sval = sval;
            return val;
        }
    };

    std::ostream& operator<<(std::ostream & out, const AST::value & val);
    bool operator==(const AST::value &, const AST::value &);

    class MiniCalcSyntaxError : public std::logic_error {
    public:
        using std::logic_error::logic_error;
    };

    class MiniCalcEvaluationError : public std::logic_error {
    public:
        MiniCalcEvaluationError(const std::string & msg) : std::logic_error(msg) {}
    };

    class MiniCalcEvaluationTypeError : public MiniCalcEvaluationError {
    public:
        MiniCalcEvaluationTypeError(const std::string & msg) : MiniCalcEvaluationError(msg) {}
    };

    //! \brief Base class of an abstract syntax tree node.
    struct node {
        node() {}

        //! \brief Statically typecheck the abstract syntax subtree.
        virtual value_type type_check(bool unbound_ok) const = 0;

        //! \brief Evaluate the abstract syntax subtree.
        virtual value evaluate() const = 0;

        virtual ~node() = default;
    };

    node* create_node(const std::string & variable_name, MiniCalc * owner);
    node* create_node(value val);
    node* create_node(const std::string op, std::deque<std::shared_ptr<node>> &&args);

};
