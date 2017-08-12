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

#include "ast.h"
#include <functional>
#include <stdexcept>
#include "minicalc.h"
#include "../../assert.h"

namespace AST {

    std::ostream& operator<<(std::ostream & out, const AST::value & val) {
        switch (val.type) {
            case AST::value_type::UNBOUND:
                out << "UNBOUND";
                break;
            case AST::value_type::BOOL:
                out << (val.bval ? "true" : "false");
                break;
            case AST::value_type::INT:
                out << val.ival;
                break;
            case AST::value_type::STRING:
                out << "\"" << val.sval << "\"";
                break;
        }
        return out;
    }

    bool operator==(const AST::value & lhs, const AST::value & rhs) {
        if (lhs.type != rhs.type) {
            return false;
        }
        switch (lhs.type) {
            case AST::value_type::UNBOUND:
                return true;
            case AST::value_type::BOOL:
                return lhs.bval == rhs.bval;
            case AST::value_type::INT:
                return lhs.ival == rhs.ival;
            case AST::value_type::STRING:
                return lhs.sval == rhs.sval;
        }
        assert (false && "Should never get here.");
    }


    value::value(const value& rhs) {
        *this = rhs;
    }

    value & value::operator=(const value& rhs) {
        if (this->type == AST::value_type::STRING
                && rhs.type != AST::value_type::STRING ) {
            this->sval.clear();
        }

        this->type = rhs.type;

        switch(rhs.type) {
            case AST::value_type::BOOL:
                this->bval = rhs.bval;
                break;
            case AST::value_type::INT:
                this->ival= rhs.ival;
                break;
            case AST::value_type::STRING:
                this->sval = rhs.sval;
                break;
            default:
                break;
        }

        return *this;
    }

    std::string to_string(const AST::value_type & type) {
        switch(type) {
            case AST::value_type::BOOL:
                return "bool";
            case AST::value_type::INT:
                return "int";
            case AST::value_type::STRING:
                return "string";
            case AST::value_type::UNBOUND:
                return "unbound";
            default:
                assert(false && "Should never get here.");
        }
    }

    //! \brief Class representing a constant value in the ast.
    class value_node : public node {
    public:
        value_node(value val) :
                val(val) {}

        value_type type_check(bool unbound_ok) const override {
            XYUV_ASSERT(val.type != value_type::UNBOUND);
            return val.type;
        }

        value evaluate() const {
            return val;
        }

        ~value_node() = default;

    private:
        value val;
    };

    //! \brief Class representing a named variable in the ast.
    class variable_node : public node {
    public:
        variable_node(const std::string &name, const MiniCalc &owner) :
                owner(owner),
                name(name) {}

        value_type type_check(bool unbound_ok) const override {
            const value *variable = owner.get_variable(name);
            if (!variable) {
                if (unbound_ok) {
                    return value_type::UNBOUND;
                } else {
                    throw MiniCalcEvaluationError("Unknown identifier '" + name + "'. No such constant or variable.");
                }
            }
            return variable->type;
        }

        value evaluate() const override {
            const value *variable = owner.get_variable(name);
            if (!variable) {
                throw MiniCalcEvaluationError("Unknown identifier '" + name + "'. No such constant or variable.");
            }
            return *variable;
        }

    private:
        const MiniCalc &owner;
        const std::string name;
    };

    node* create_node(const std::string & variable_name, MiniCalc * owner) {
        XYUV_ASSERT(owner != nullptr && "Owner must be set.");
        return new variable_node(variable_name, *owner);
    }

    node* create_node(value val) {
        return new value_node(val);
    }

};
