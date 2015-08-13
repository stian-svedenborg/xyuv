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

class MiniCalc;

//! \file File contains abstract syntax tree definitions and helper functions.

//! \brief Base class of an abstract syntax tree node.
struct node {
    //! \brief Evaluate the abstract syntax subtree.
    virtual int evaluate() const = 0;
    virtual ~node() = default;
};

//! \brief Allocate a new node which applies func(arg) on the sub-tree.
node* create_node(node* arg, std::function<int(int)> func);

//! \brief Allocate a new node which applies func(arg0, arg1) on the sub-trees.
node* create_node(node* arg0, node* arg1, std::function<int(int, int)> func);

//! \brief Allocate a new node representing a single constant \a value, this is a leaf node.
node* create_node(int value);

//! \brief Allocate a new node representing a single named \a variable, this is a leaf node.
//! \param owner Reference to xyuv::MiniCalc object in which to lookup the value of variable during evaluation.
node* create_node(const std::string & variable, const MiniCalc & owner);

