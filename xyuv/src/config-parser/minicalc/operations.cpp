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

#include <stdexcept>
#include <unordered_map>
#include <functional>
#include <deque>
#include <memory>
#include <vector>
#include <sstream>
#include "minicalc.h"
#include "ast.h"
#include "../../assert.h"
#include "../../to_string.h"

namespace AST {

    std::ostream& print_function_signature(std::ostream & out, const std::string & fname, const std::vector<value_type> & arg_types) {
        out << fname << "(";
        for (uint32_t i = 0; i < arg_types.size(); i++) {
            if (i != 0) {
                out << ", ";
            }
            out << to_string(arg_types[i]);
        }
        out << ")";

        return out;
    }


    //! Interface for operations. All implementors must be reentrant.
class Operation {
public:


    //! \brief Type check the operation. Throwing MiniCalcEvaluationError on failures.
    //! \param unbound_ok: If unbound_ok is true, then do not throw MiniCalcEvaluationError if variables are unbound.
    //! \param args: List of arguments to the operation.
    //! \throw MiniCalcEvaluationError on type
    //! \return The evaluated type if evaluation was successful.
    virtual value_type type_check(bool unbound_ok, const std::deque<std::shared_ptr<AST::node>>& args) const = 0;

    //! \brief Evaluate the operation on the given arguments.
    //! \param args: List of arguments to the operation.
    //! \return Value of the expression.
    virtual value evaluate(const std::deque<std::shared_ptr<AST::node>>& args) const = 0;

};




//! \brief Abstract class for handling type_checking of binary integer operations.
class BinaryOperator : public Operation {
public:

    struct SupportedTypes {
        value_type return_type;
        value_type lhs_type;
        value_type rhs_type;
    };

    BinaryOperator(const std::string & op, std::vector<SupportedTypes> && supported_types) :
            op(op),
            supported_types(std::move(supported_types))
    {}

    virtual value_type type_check(bool unbound_ok, const std::deque<std::shared_ptr<AST::node>> &args) const override {
        XYUV_ASSERT(args.size() == 2);
        value_type lhs = args[0]->type_check(unbound_ok);
        value_type rhs = args[1]->type_check(unbound_ok);

        if (lhs == value_type::UNBOUND || rhs == value_type::UNBOUND) {
            XYUV_ASSERT(unbound_ok && "Should never get here unless unbound_ok is set.");
            return value_type::UNBOUND;
        }

        for (const auto & supported_type : supported_types) {
            if (supported_type.lhs_type == lhs && supported_type.rhs_type == rhs) {
                return supported_type.return_type;
            }
        }

        throw MiniCalcEvaluationTypeError("Operation '" + op + "' not supported for operands of type '"
                                          + to_string(lhs) + "' and '" + to_string(rhs) + "'.");
    }

private:
    std::string op;
    std::vector<SupportedTypes> supported_types;
};

    class Plus : public BinaryOperator {
    public:
        Plus() :
                BinaryOperator(
                        "+", {
                                {value_type::INT, value_type::INT, value_type::INT},
                                {value_type::STRING, value_type::STRING, value_type::STRING}
                        })
        {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();

            XYUV_ASSERT(lhs.type == rhs.type);

            switch (lhs.type) {
                case value_type::INT:
                    return value::integer(lhs.ival + rhs.ival);
                case value_type::STRING:
                    return value::string(lhs.sval + rhs.sval);
                default:
                    XYUV_ASSERT(false && "Should never get here");
            }
        }
    };

class Subtract : public BinaryOperator {
public:
    Subtract() :
            BinaryOperator("-", {{value_type::INT, value_type::INT, value_type::INT}})
    {}

private:
    virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
        value lhs = args[0]->evaluate();
        value rhs = args[1]->evaluate();
        return value::integer(lhs.ival - rhs.ival);
    }
};

    class Multiply : public BinaryOperator {
    public:
        Multiply() :
                BinaryOperator("*", {{value_type::INT, value_type::INT, value_type::INT}})
        {}

    private:
        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();
            return value::integer(lhs.ival * rhs.ival);
        }
    };


    class Divide : public BinaryOperator {
    public:
        Divide() :
                BinaryOperator("/", {{value_type::INT, value_type::INT, value_type::INT}})
        {}

    private:
        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();
            if (rhs.ival == 0) {
                throw MiniCalcEvaluationError("Division by Zero.");
            }
            return value::integer(lhs.ival / rhs.ival);
        }
    };

    class Modulo : public BinaryOperator {
    public:
        Modulo() :
                BinaryOperator("%", {{value_type::INT, value_type::INT, value_type::INT}})
        {}

    private:
        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();
            if (rhs.ival == 0) {
                throw MiniCalcEvaluationError("Division by Zero.");
            }
            return value::integer(lhs.ival % rhs.ival);
        }
    };

    class Pow : public BinaryOperator {
    public:
        Pow() :
                BinaryOperator("**", {{value_type::INT, value_type::INT, value_type::INT}})
        {}

    private:
        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value base = args[0]->evaluate();
            value exponent = args[1]->evaluate();
            value result = value::integer(1);
            while (exponent.ival > 0) {
                result.ival *= base.ival;
                --exponent.ival;
            }
            return result;
        }
    };

    class LogicAnd : public BinaryOperator {
    public:
        LogicAnd() :
                BinaryOperator("&&", {{value_type::BOOL, value_type::BOOL, value_type::BOOL}})
        {}

    private:
        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();

            return value::boolean(lhs.bval && rhs.bval);
        }
    };

    class LogicOr : public BinaryOperator {
    public:
        LogicOr() :
                BinaryOperator("||", {{value_type::BOOL, value_type::BOOL, value_type::BOOL}})
        {}

    private:
        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();

            return value::boolean(lhs.bval || rhs.bval);
        }
    };


    class LogicEq : public BinaryOperator {
    public:
        LogicEq() :
                BinaryOperator(
                        "==", {
                                {value_type::BOOL, value_type::BOOL, value_type::BOOL},
                                {value_type::BOOL, value_type::INT, value_type::INT},
                                {value_type::BOOL, value_type::STRING, value_type::STRING}
                })
        {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();

            XYUV_ASSERT(lhs.type == rhs.type);

            switch (lhs.type) {
                case value_type::INT:
                    return value::boolean(lhs.ival == rhs.ival);
                case value_type::BOOL:
                    return value::boolean(lhs.bval == rhs.bval);
                case value_type::STRING:
                    return value::boolean(lhs.sval == rhs.sval);
                default:
                    XYUV_ASSERT(false && "Should never get here");
            }
        }
    };

    class LogicNE : public BinaryOperator {
    public:
        LogicNE() :
                BinaryOperator(
                        "!=", {
                                {value_type::BOOL, value_type::BOOL, value_type::BOOL},
                                {value_type::BOOL, value_type::INT, value_type::INT},
                                {value_type::BOOL, value_type::STRING, value_type::STRING}
                        })
        {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();

            XYUV_ASSERT(lhs.type == rhs.type);

            switch (lhs.type) {
                case value_type::INT:
                    return value::boolean(lhs.ival != rhs.ival);
                case value_type::BOOL:
                    return value::boolean(lhs.bval != rhs.bval);
                case value_type::STRING:
                    return value::boolean(lhs.sval != rhs.sval);
                default:
                    XYUV_ASSERT(false && "Should never get here");
            }
        }
    };

    class LogicLT : public BinaryOperator {
    public:
        LogicLT() :
                BinaryOperator(
                        "<", {
                                {value_type::BOOL, value_type::INT, value_type::INT}
                        })
        {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();

            XYUV_ASSERT(lhs.type == rhs.type);

            switch (lhs.type) {
                case value_type::INT:
                    return value::boolean(lhs.ival < rhs.ival);
                default:
                    XYUV_ASSERT(false && "Should never get here");
            }
        }
    };

    class LogicLE : public BinaryOperator {
    public:
        LogicLE() :
                BinaryOperator(
                        "<=", {
                                {value_type::BOOL, value_type::INT, value_type::INT}
                        })
        {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();

            XYUV_ASSERT(lhs.type == rhs.type);

            switch (lhs.type) {
                case value_type::INT:
                    return value::boolean(lhs.ival <= rhs.ival);
                default:
                    XYUV_ASSERT(false && "Should never get here");
            }
        }
    };

    class LogicGT : public BinaryOperator {
    public:
        LogicGT() :
                BinaryOperator(
                        ">", {
                                {value_type::BOOL, value_type::INT, value_type::INT}
                        })
        {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();

            XYUV_ASSERT(lhs.type == rhs.type);

            switch (lhs.type) {
                case value_type::INT:
                    return value::boolean(lhs.ival > rhs.ival);
                default:
                    XYUV_ASSERT(false && "Should never get here");
            }
        }
    };

    class LogicGE : public BinaryOperator {
    public:
        LogicGE() :
                BinaryOperator(
                        ">=", {
                                {value_type::BOOL, value_type::INT, value_type::INT}
                        })
        {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();

            XYUV_ASSERT(lhs.type == rhs.type);

            switch (lhs.type) {
                case value_type::INT:
                    return value::boolean(lhs.ival >= rhs.ival);
                default:
                    XYUV_ASSERT(false && "Should never get here");
            }
        }
    };

    //////////////////////////////////
//! \brief Abstract class for handling type_checking of binary integer operations.
    class UnaryOperator : public Operation {
    public:

        struct SupportedTypes {
            value_type return_type;
            value_type arg_type;
        };

        UnaryOperator(const std::string & op, std::vector<SupportedTypes> && supported_types) :
                op(op),
                supported_types(std::move(supported_types))
        {}

        virtual value_type type_check(bool unbound_ok, const std::deque<std::shared_ptr<AST::node>> &args) const override {
            XYUV_ASSERT(args.size() == 1);
            value_type arg = args[0]->type_check(unbound_ok);

            if (arg == value_type::UNBOUND) {
                XYUV_ASSERT(unbound_ok && "Should never get here unless unbound_ok is set.");
                return value_type::UNBOUND;
            }

            for (const auto & supported_type : supported_types) {
                if (supported_type.arg_type == arg) {
                    return supported_type.return_type;
                }
            }

            throw MiniCalcEvaluationTypeError("Unary operation '" + op + "' not supported for operand of type '"
                                              + to_string(arg) + "'.");
        }

    private:
        std::string op;
        std::vector<SupportedTypes> supported_types;
    };


    class NegateInt : public UnaryOperator {
    public:
        NegateInt() : UnaryOperator("-", {{value_type::INT, value_type::INT}}) {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value arg = args[0]->evaluate();
            return value::integer(-arg.ival);
        }
    };

    class NegateBool : public UnaryOperator {
    public:
        NegateBool() : UnaryOperator("!", {{value_type::BOOL, value_type::BOOL}}) {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value arg = args[0]->evaluate();
            return value::boolean(!arg.bval);
        }
    };


    //////////////////////////////////////////////////////////////

    class FunctionCall : public Operation {
    public:
        struct SupportedTypes {
            value_type return_type;
            std::vector<value_type> arg_types;
        };

        FunctionCall(const std::string &fname, std::vector<SupportedTypes> && supported_types) :
                fname(fname),
                supported_types(supported_types)
        {}

        virtual value_type type_check(bool unbound_ok, const std::deque<std::shared_ptr<AST::node>> &args) const override {
            std::vector<value_type> arg_types;
            for (const auto& arg : args) {
                value_type type = arg->type_check(unbound_ok);
                if (type == value_type::UNBOUND) {
                    XYUV_ASSERT(unbound_ok && "Should never get here unless unbound_ok is set.");
                    return value_type::UNBOUND;
                }
                arg_types.push_back(type);
            }

            for (const auto & supported_type : supported_types) {
                if (supported_type.arg_types == arg_types) {
                    return supported_type.return_type;
                }
            }

            std::ostringstream error_sstr;

            error_sstr << "No such function ";
            print_function_signature(error_sstr, arg_types);
            error_sstr << ")\ncandidates are:\n";
            for (const auto & supported_type : supported_types) {
                error_sstr << "    ";
                print_function_signature(error_sstr, supported_type.arg_types);
            }

            throw MiniCalcEvaluationTypeError(error_sstr.str());
        }

    private:

        void print_function_signature(std::ostream & out, const std::vector<value_type> & arg_types) const {
            AST::print_function_signature(out, this->fname, arg_types);
        }

        std::string fname;
        std::vector<SupportedTypes> supported_types;
    };

    class NextMultiple : public FunctionCall {
    public:
        NextMultiple() : FunctionCall("next_multiple", {{ value_type::INT, {value_type::INT, value_type::INT}}}) {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value base = args[0]->evaluate();
            value multiplier = args[1]->evaluate();

            if (multiplier.ival < 1) {
                std::ostringstream err_sstr;
                err_sstr << "Domain Error: next_multiple(base=" << base.ival << ", multiplier=" << multiplier.ival << ")"
                         << " multiplier must be >= 1";
                throw MiniCalcEvaluationError(err_sstr.str());
            }

            return value::integer(((base.ival + (multiplier.ival-1)) / multiplier.ival)*multiplier.ival);
        }
    };

    class Abs : public FunctionCall {
    public:
        Abs() : FunctionCall("abs", {{ value_type::INT, {value_type::INT}}}) {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value arg = args[0]->evaluate();

            return arg.ival < 0 ? value::integer(-arg.ival) : arg;
        }
    };


    //// Helpers
    using ival_t = AST::value::ival_t;
    ival_t gcd(ival_t lhs, ival_t rhs) {
        if (lhs > rhs) {
            return gcd(lhs-rhs, rhs);
        } else if (lhs < rhs) {
            return gcd(lhs, rhs-lhs);
        }
        else {
            return lhs;
        }
    }

    class GCD : public FunctionCall {
    public:
        GCD() : FunctionCall("gcd", {{ value_type::INT, {value_type::INT, value_type::INT}}}) {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();

            if (lhs.ival <= 0 || rhs.ival <=0 ) {
                std::ostringstream err_sstr;
                err_sstr << "Domain Error: gcd(a=" << lhs.ival << ", b=" << rhs.ival << ")"
                         << " gcd() must have positive, non-zero operands.";
                throw MiniCalcEvaluationError(err_sstr.str());
            }

            return value::integer(gcd(lhs.ival, rhs.ival));
        }

    };

    class LCM : public FunctionCall {
    public:
        LCM() : FunctionCall("lcm", {{ value_type::INT, {value_type::INT, value_type::INT}}}) {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value lhs = args[0]->evaluate();
            value rhs = args[1]->evaluate();

            if (lhs.ival <= 0 || rhs.ival <=0 ) {
                std::ostringstream err_sstr;
                err_sstr << "Domain Error: lcm(a=" << lhs.ival << ", b=" << rhs.ival << ")"
                         << " lcm() must have positive, non-zero operands.";
                throw MiniCalcEvaluationError(err_sstr.str());
            }

            return value::integer( (lhs.ival * rhs.ival) / gcd(lhs.ival, rhs.ival));
        }

    };


    /////// Casts
    class CastInt : public FunctionCall {
    public:
        CastInt() : FunctionCall("int", {
                { value_type::INT, {value_type::INT}},
                { value_type::INT, {value_type::STRING}},
                { value_type::INT, {value_type::BOOL}},
        }) {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value val = args[0]->evaluate();

            switch (val.type) {
                case value_type::INT:
                    return val;
                case value_type::BOOL:
                    return value::integer(val.bval ? 1 : 0);
                case value_type::STRING:
                    {
                        char * endptr = nullptr;
                        ival_t ival = ival_t(std::strtoll(val.sval.c_str(), &endptr, 10));
                        if (endptr != (val.sval.c_str() + val.sval.size())) {
                            std::ostringstream error_sstr;
                            error_sstr << "int('" + val.sval + "') Could not cast '" << val.sval
                                       << "' to int. It is not a base10 number. (Spaces and other characters are not allowed).";
                            throw MiniCalcEvaluationError(error_sstr.str());
                        }
                        return value::integer(ival);
                    }
                default:
                    XYUV_ASSERT(false && "Should never get here.");
            }
        }
    };


    class CastBool : public FunctionCall {
    public:
        CastBool() : FunctionCall("bool", {
                { value_type::BOOL, {value_type::INT}},
                { value_type::BOOL, {value_type::STRING}},
                { value_type::BOOL, {value_type::BOOL}},
        }) {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value val = args[0]->evaluate();

            switch (val.type) {
                case value_type::INT:
                    return value::boolean(val.ival != 0);
                case value_type::BOOL:
                    return val;
                case value_type::STRING:
                {
                    return value::boolean(val.sval != "");
                }
                default:
                    XYUV_ASSERT(false && "Should never get here.");
            }
        }
    };

    class CastString : public FunctionCall {
    public:
        CastString() : FunctionCall("str", {
                { value_type::STRING, {value_type::INT}},
                { value_type::STRING, {value_type::STRING}},
                { value_type::STRING, {value_type::BOOL}},
        }) {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value val = args[0]->evaluate();

            switch (val.type) {
                case value_type::INT:
                    return value::string(xyuv::to_string(val.ival));
                case value_type::BOOL:
                    return value::string(val.bval ? "true" : "false");
                case value_type::STRING:
                    return val;
                default:
                    XYUV_ASSERT(false && "Should never get here.");
            }
        }
    };

    class If : public FunctionCall {
    public:
        If() : FunctionCall("if", {
                { value_type::INT, {value_type::BOOL, value_type::INT, value_type::INT}},
                { value_type::BOOL, {value_type::BOOL, value_type::BOOL, value_type::BOOL}},
                { value_type::STRING, {value_type::BOOL, value_type::STRING, value_type::STRING}},
        }) {}

        virtual value evaluate(const std::deque<std::shared_ptr<AST::node>> &args) const override {
            value test = args[0]->evaluate();
            if (test.bval) {
                return args[1]->evaluate();
            } else {
                return args[2]->evaluate();
            }
        }
    };


    class OperationNode : public node {
    public:
        OperationNode(std::shared_ptr<Operation> op, std::deque<std::shared_ptr<node>> && args) :
            args(std::move(args)),
            op(op)
        {}

        value_type type_check(bool unbound_ok) const override {
            return op->type_check(unbound_ok, args);
        }

        value evaluate() const override {
            value_type type = op->type_check(false, args);
            value val = op->evaluate(args);
            XYUV_ASSERT(val.type == type && "Error, inconsistent type.");
            return val;
        }



    private:
        std::deque<std::shared_ptr<node>> args;
        std::shared_ptr<Operation> op;
    };

    node* create_node(const std::string op, std::deque<std::shared_ptr<node>> &&args) {
        static const std::unordered_map<std::string, std::shared_ptr<Operation>> supported_operation = {
                // Operators
                {"+", std::shared_ptr<Operation>(new Plus())},
                {"-", std::shared_ptr<Operation>(new Subtract())},
                {"*", std::shared_ptr<Operation>(new Multiply())},
                {"/", std::shared_ptr<Operation>(new Divide())},
                {"%", std::shared_ptr<Operation>(new Modulo())},
                {"**", std::shared_ptr<Operation>(new Pow())},
                {"-Unary", std::shared_ptr<Operation>(new NegateInt())},
                {"!", std::shared_ptr<Operation>(new NegateBool())},
                {"==", std::shared_ptr<Operation>(new LogicEq())},
                {"!=", std::shared_ptr<Operation>(new LogicNE())},
                {"<", std::shared_ptr<Operation>(new LogicLT())},
                {"<=", std::shared_ptr<Operation>(new LogicLE())},
                {">", std::shared_ptr<Operation>(new LogicGT())},
                {">=", std::shared_ptr<Operation>(new LogicGE())},
                {"&&", std::shared_ptr<Operation>(new LogicAnd())},
                {"||", std::shared_ptr<Operation>(new LogicOr())},

                // Functions
                {"next_multiple", std::shared_ptr<Operation>(new NextMultiple())},
                {"lcm", std::shared_ptr<Operation>(new LCM())},
                {"gcd", std::shared_ptr<Operation>(new GCD())},
                {"abs", std::shared_ptr<Operation>(new Abs())},

                // Casts
                {"int", std::shared_ptr<Operation>(new CastInt())},
                {"bool", std::shared_ptr<Operation>(new CastBool())},
                {"str", std::shared_ptr<Operation>(new CastString())},

                // Conditional
                {"if", std::shared_ptr<Operation>(new If())},
        };

        const auto it = supported_operation.find(op);
        if (it == supported_operation.end()) {
            throw MiniCalcSyntaxError("No such function: '" + op + "()'");
        }

        return new OperationNode(it->second, std::move(args));
    }

}