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

#include "minicalc.h"
#include "parser.h"
#include "ast.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

MiniCalc::MiniCalc(const std::string & expression)
: expression(expression)
{
    parse_expression(expression);
}

MiniCalc::~MiniCalc() = default;

const uint64_t * MiniCalc::get_variable( const std::string & variable_name ) const {
    if (!variables) {
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


uint64_t MiniCalc::evaluate(const std::unordered_map<std::string, uint64_t> * variables) const {

    this->variables = variables;

    if (!parse_errors.empty()) {
        std::cout << "Parse error in expression: '" << expression << "'" << std::endl;
        for (auto & msg : parse_errors ) {
            std::cout << "  " << msg << std::endl;
        }
        return -1;
    }

    // Clear old runtime_errors
    runtime_errors.clear();
    int result = -1;
    try {
        result = root->evaluate();
    } catch( std::runtime_error & e ) {
        runtime_error(e.what());
    }

    // If the evaluation encountered run-time errors they are recorded here.
    if (!runtime_errors.empty()) {
        std::cout << "Could not evaluate expression: '" << expression << "'" << std::endl;
        for (auto & msg : runtime_errors ) {
            std::cout << "  " << msg << std::endl;
        }
        return -1;
    }

    return result;
}

void MiniCalc::set_root(node *node) {
    root.reset(node);
}

void MiniCalc::parse_error(const std::string & msg) const {
    parse_errors.push_back(msg);
}

void MiniCalc::runtime_error(const std::string & msg) const {
    runtime_errors.push_back(msg);
}

int MiniCalc::parse_expression(const std::string & expression)
{
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

	"+"			{   Parse(parser, TOK_PLUS, nullptr, this);continue;}
    "-"			{   Parse(parser, TOK_MINUS, nullptr, this);continue;}
    "*"			{   Parse(parser, TOK_MUL, nullptr, this);continue;}
    "/"			{   Parse(parser, TOK_DIV, nullptr, this); continue;}
    "%"			{   Parse(parser, TOK_MOD, nullptr, this); continue;}
    "**"        {   Parse(parser, TOK_POW, nullptr, this); continue; }
    "("         {   Parse(parser, TOK_LPAREN, nullptr, this); continue; }
    ")"         {   Parse(parser, TOK_RPAREN, nullptr, this); continue; }
    ","         {   Parse(parser, TOK_COMMA, nullptr, this); continue; }


    WS          {   continue; }
    "\000"		{
                    Parse(parser, 0, nullptr, this);
                    break;
                }
	[^]		    {	errorstate = 1; break; }
*/

	}

	ParseFree(parser, free);

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
        MiniCalc expr(argv[1]);
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
