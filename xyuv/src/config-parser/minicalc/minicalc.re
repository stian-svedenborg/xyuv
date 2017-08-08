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

#include "minicalc.h"
#include "parser.h"
#include "ast.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <sstream>

MiniCalc::MiniCalc(const std::string & expression, const std::set<std::string> * constants)
: constants(nullptr)
, expression(expression)
{
    parse_expression(expression, constants);
}

MiniCalc::~MiniCalc() = default;

const AST::value * MiniCalc::get_variable( const std::string & variable_name ) const {
    if (this->variables == nullptr ) {
        return nullptr;
    }
    else {
        auto it = variables->find(variable_name);
        if (it == variables->end()) {
            return nullptr;
        }
        else {
            return &(it->second);
        }
    }
}

AST::value MiniCalc::evaluate(const std::unordered_map<std::string, AST::value> * variables) const {

    this->variables = variables;

    if (!parse_errors.empty()) {
        std::ostringstream sout;
        sout << "Parse error in expression: '" << expression << "'" << std::endl;
        for (auto & msg : parse_errors ) {
            sout << "  " << msg << std::endl;
        }
        throw MiniCalcParseError(sout.str());
    }

    // Clear old runtime_errors
    runtime_errors.clear();
    AST::value result;
    try {
        result = root->evaluate();
    } catch( std::runtime_error & e ) {
        runtime_error(e.what());
    }

    // If the evaluation encountered run-time errors they are recorded here.
    if (!runtime_errors.empty()) {
        std::ostringstream sout;
        sout << "Could not evaluate expression: '" << expression << "'" << std::endl;
        for (auto & msg : runtime_errors ) {
            sout << "  " << msg << std::endl;
        }
        throw MiniCalcParseError(sout.str());
    }

    return result;
}

void MiniCalc::register_dependency(AST::variable_node *node) {
    this->dependencies[node->get_name()].push_back(node);
}

void MiniCalc::set_root(std::shared_ptr<AST::node> node) {
    root = node;
}

void MiniCalc::parse_error(const std::string & msg) const {
    parse_errors.push_back(msg);
}

void MiniCalc::runtime_error(const std::string & msg) const {
    runtime_errors.push_back(msg);
}

int MiniCalc::parse_expression(const std::string & expression, const std::set<std::string> * constants)
{
    this->constants = constants;

    #define YYCTYPE         char
    int errorstate = 0;
	const char *YYCURSOR = expression.c_str();
	const char * const YYLIMIT  = YYCURSOR + expression.size() + 1 ;
    //const char * YYMARKER = NULL;
    //(void)YY


#define YYFILL(n)		{  }

    void * parser = ParseAlloc(malloc);

	for(;;)
	{

	    const char *token_begin = YYCURSOR;
/*!re2c

	DIGIT       = [0-9] ;
	INTEGER	    = DIGIT+ ;
    ALPHA       = [a-zA-Z] ;
    IDENTIFIER  = ALPHA (ALPHA|DIGIT|[_]|[.\]\[])* ;
    WS		    = [ \t]+ ;

    "true"      {   Parse(parser, TOK_TRUE, nullptr, this); continue;}
    "false"     {   Parse(parser, TOK_FALSE, nullptr, this); continue;}

	INTEGER	    {
	                char * lend = NULL;
	                Token * tok = new Token;
                    tok->value = (int)strtol(token_begin, &lend, 10);
	                assert(lend == YYCURSOR);

	                Parse(parser, TOK_INT, tok, this);
	                continue;
	            }

    IDENTIFIER  {
        Token * tok = new Token;
        tok->identifier = std::string(token_begin, YYCURSOR);
        Parse(parser, TOK_IDENTIFIER, tok, this);
        continue;
    }

	"+"			{   Parse(parser, TOK_PLUS, nullptr, this); continue;}
    "-"			{   Parse(parser, TOK_MINUS, nullptr, this); continue;}
    "*"			{   Parse(parser, TOK_MUL, nullptr, this); continue;}
    "/"			{   Parse(parser, TOK_DIV, nullptr, this); continue;}
    "%"			{   Parse(parser, TOK_MOD, nullptr, this); continue;}
    "**"        {   Parse(parser, TOK_POW, nullptr, this); continue; }
    "("         {   Parse(parser, TOK_LPAREN, nullptr, this); continue; }
    ")"         {   Parse(parser, TOK_RPAREN, nullptr, this); continue; }
    ","         {   Parse(parser, TOK_COMMA, nullptr, this); continue; }

    "=="        {   Parse(parser, TOK_LOGIC_EQ, nullptr, this); continue; }
    "!="        {   Parse(parser, TOK_LOGIC_NE, nullptr, this); continue; }
    "<"         {   Parse(parser, TOK_LOGIC_LT, nullptr, this); continue; }
    ">"         {   Parse(parser, TOK_LOGIC_GT, nullptr, this); continue; }
    "<="        {   Parse(parser, TOK_LOGIC_LE, nullptr, this); continue; }
    ">="        {   Parse(parser, TOK_LOGIC_GE, nullptr, this); continue; }
    "!"         {   Parse(parser, TOK_LOGIC_NEG, nullptr, this); continue; }
    "&&"        {   Parse(parser, TOK_LOGIC_AND, nullptr, this); continue; }
    "||"        {   Parse(parser, TOK_LOGIC_OR, nullptr, this); continue; }


    WS          {   continue; }
    "\000"		{
                    Parse(parser, 0, nullptr, this);
                    break;
                }
	[^]		    {	errorstate = 1; break; }
*/

	}

	ParseFree(parser, free);

    this->constants = nullptr;

	return errorstate;
}

// The resulting scanner can also be used as a standalone program to ease debugging.
#ifdef COMPILE_TEST_PROGRAM
#include <iostream>

int main(int argc, char **argv)
{
    using namespace std;

    char tmp[] =  "Trace: ";
    FILE * file = fopen("trace.txt", "w");
    ParseTrace(file, tmp);

    std::unordered_map<std::string, uint64_t> variables = {
        { "var1", 1},
        { "var2", 2},
        { "var3", 3},
    };

	if (argc > 1)
	{
        MiniCalc expr(argv[1], nullptr);
        cout << expr.evaluate(nullptr) << endl;

        return 0;
	}
	else
	{
	    std::cout << "error" << endl;
		return 1;
	}

    fclose(file);
}

#endif
