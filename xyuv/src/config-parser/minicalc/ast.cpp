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

#include "ast.h"
#include <functional>
#include <stdexcept>
#include "minicalc.h"

//! \brief Class representing a unary operation in the ast.
class unary_node : public node {
public:
    unary_node(node* expression, std::function<int(int)> operation) :
            expression(expression),
            operation(operation)
    {}

    int evaluate() const {
        return operation(expression->evaluate());
    }
    virtual ~unary_node() {
        delete expression;
    }

private:
    node* expression;
    std::function<int(int)> operation;
};

//! \brief Class representing a binary operation in the ast.
class binary_node : public node {
public:
    binary_node(node *lhs, node *rhs, std::function<int(int, int)> operation) :
            lhs(lhs),
            rhs(rhs),
            operation(operation)
    {}

    int evaluate() const {
        return operation(lhs->evaluate(), rhs->evaluate());
    }
    virtual ~binary_node() {
        delete lhs;
        delete rhs;
    }
private:
    node *lhs, *rhs;
    std::function<int(int, int)> operation;
};

//! \brief Class representing a constant value in the ast.
class value_node : public node {
public:
    value_node(int value) :
            value(value)
    {}

    int evaluate() const {
        return value;
    }
    ~value_node() = default;
private:
    int value;
};

//! \brief Class representing a named variable in the ast.
class variable_node : public node {
public:
    variable_node(const std::string & name, const MiniCalc & owner) :
        owner(owner),
        name(name)
    {}

    int evaluate() const {
        const uint64_t * variable = owner.get_variable(name);
        if (!variable) {
            throw std::runtime_error("Unknown variable '" + name + "'.");
        }
        return *variable;
    }
private:
    const MiniCalc & owner;
    const std::string name;
};


// Factory functions
node* create_node(node* child, std::function<int(int)> func) {
    return new unary_node(child, func);
}
node* create_node(node* lhs, node* rhs, std::function<int(int, int)> func){
    return new binary_node(lhs, rhs, func);
}
node* create_node(int val) {
    return new value_node(val);
}
node* create_node(const std::string & name, const MiniCalc & owner) {
    return new variable_node(name, owner);
}



