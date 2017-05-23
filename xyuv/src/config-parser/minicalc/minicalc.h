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
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <set>

namespace AST {
    struct node;
    struct value;
};

//! Evaluate \a expression given the set of variable values \a variables.
extern AST::value minicalc_evaluate(const std::string & expression, std::unordered_map<std::string, AST::value> variables);

class MiniCalcParseError : public std::runtime_error {
public:
    MiniCalcParseError(const std::string & msg) : std::runtime_error(msg) {}
};

//! Class containing the MiniCalc (great name isn't it) expression parser.
class MiniCalc {
public:
    //! Parse \a expression and construct parse tree which is later evaluated.
    //! TODO: Describe error handling.
    MiniCalc(const std::string & expression);

    ~MiniCalc();

    //! Evaluate the expression given the defined \a variables. If variables = nullptr,
    //! then it assumed that no variables are defined.
    AST::value evaluate(const std::unordered_map<std::string, AST::value> * variables) const;

    //! Type declarations private to MiniCalc and parser
    struct Token {
        int value; //! Constant value
        std::string identifier; //! Variable or function name.
    };

    // The following functions dictate the interface to the parser.
    //! Set the root of the parse tree in *this.
    void set_root(std::shared_ptr<AST::node> node);
    //! Get the value of a variable as owned by *this.
    //! \return nullptr if the value does not exist, otherwise a valid pointer to the value of \a variable
    const AST::value * get_variable( const std::string & variable ) const;

    //! Push a parsing error message.
    void parse_error( const std::string & msg ) const;

    //! Push a run-time error message. (This message stack is cleared each time xyuv::evaluate() is called.
    void runtime_error( const std::string & msg ) const;

    void register_dependency(const std::string var_name) {
        this->dependencies.insert(var_name);
    }

    const std::set<std::string> & get_dependencies() const {
        return dependencies;
    }

private:
    //! Parse \a expression and assign the result to root.
    int parse_expression(const std::string & expression);

    // Private variables.
    //! Symbol table of all variables in the expression.
    mutable const std::unordered_map<std::string, AST::value > *variables = nullptr;

    //! Variable names this expression depend on.
    std::set<std::string> dependencies;

    //! Vectors temporarily holding parse and runtime_errors.
    mutable std::vector<std::string> parse_errors, runtime_errors;

    //! Root of the AST.
    std::shared_ptr<AST::node> root;

    //! Copy of the expression for logging/debugging purposes.
    std::string expression;
};
