/* Generated by re2c 0.16 on Wed Aug  9 21:51:03 2017 */
#line 1 "minicalc.re"
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

#line 132 "<stdout>"
{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 6) YYFILL(6);
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy2;
	case '\t':
	case ' ':	goto yy6;
	case '!':	goto yy9;
	case '%':	goto yy11;
	case '&':	goto yy13;
	case '(':	goto yy14;
	case ')':	goto yy16;
	case '*':	goto yy18;
	case '+':	goto yy20;
	case ',':	goto yy22;
	case '-':	goto yy24;
	case '/':	goto yy26;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy28;
	case '<':	goto yy31;
	case '=':	goto yy33;
	case '>':	goto yy34;
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy36;
	case 'f':	goto yy39;
	case 't':	goto yy40;
	case '|':	goto yy41;
	default:	goto yy4;
	}
yy2:
	++YYCURSOR;
#line 178 "minicalc.re"
	{
                    Parse(parser, 0, nullptr, this);
                    break;
                }
#line 226 "<stdout>"
yy4:
	++YYCURSOR;
yy5:
#line 182 "minicalc.re"
	{	errorstate = 1; break; }
#line 232 "<stdout>"
yy6:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case '\t':
	case ' ':	goto yy6;
	default:	goto yy8;
	}
yy8:
#line 177 "minicalc.re"
	{   continue; }
#line 245 "<stdout>"
yy9:
	++YYCURSOR;
	switch ((yych = *YYCURSOR)) {
	case '=':	goto yy42;
	default:	goto yy10;
	}
yy10:
#line 172 "minicalc.re"
	{   Parse(parser, TOK_LOGIC_NEG, nullptr, this); continue; }
#line 255 "<stdout>"
yy11:
	++YYCURSOR;
#line 160 "minicalc.re"
	{   Parse(parser, TOK_MOD, nullptr, this); continue;}
#line 260 "<stdout>"
yy13:
	yych = *++YYCURSOR;
	switch (yych) {
	case '&':	goto yy44;
	default:	goto yy5;
	}
yy14:
	++YYCURSOR;
#line 162 "minicalc.re"
	{   Parse(parser, TOK_LPAREN, nullptr, this); continue; }
#line 271 "<stdout>"
yy16:
	++YYCURSOR;
#line 163 "minicalc.re"
	{   Parse(parser, TOK_RPAREN, nullptr, this); continue; }
#line 276 "<stdout>"
yy18:
	++YYCURSOR;
	switch ((yych = *YYCURSOR)) {
	case '*':	goto yy46;
	default:	goto yy19;
	}
yy19:
#line 158 "minicalc.re"
	{   Parse(parser, TOK_MUL, nullptr, this); continue;}
#line 286 "<stdout>"
yy20:
	++YYCURSOR;
#line 156 "minicalc.re"
	{   Parse(parser, TOK_PLUS, nullptr, this); continue;}
#line 291 "<stdout>"
yy22:
	++YYCURSOR;
#line 164 "minicalc.re"
	{   Parse(parser, TOK_COMMA, nullptr, this); continue; }
#line 296 "<stdout>"
yy24:
	++YYCURSOR;
#line 157 "minicalc.re"
	{   Parse(parser, TOK_MINUS, nullptr, this); continue;}
#line 301 "<stdout>"
yy26:
	++YYCURSOR;
#line 159 "minicalc.re"
	{   Parse(parser, TOK_DIV, nullptr, this); continue;}
#line 306 "<stdout>"
yy28:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy28;
	default:	goto yy30;
	}
yy30:
#line 139 "minicalc.re"
	{
	                char * lend = NULL;
	                Token * tok = new Token;
                    tok->value = (int)strtol(token_begin, &lend, 10);
	                assert(lend == YYCURSOR);

	                Parse(parser, TOK_INT, tok, this);
	                continue;
	            }
#line 335 "<stdout>"
yy31:
	++YYCURSOR;
	switch ((yych = *YYCURSOR)) {
	case '=':	goto yy48;
	default:	goto yy32;
	}
yy32:
#line 168 "minicalc.re"
	{   Parse(parser, TOK_LOGIC_LT, nullptr, this); continue; }
#line 345 "<stdout>"
yy33:
	yych = *++YYCURSOR;
	switch (yych) {
	case '=':	goto yy50;
	default:	goto yy5;
	}
yy34:
	++YYCURSOR;
	switch ((yych = *YYCURSOR)) {
	case '=':	goto yy52;
	default:	goto yy35;
	}
yy35:
#line 169 "minicalc.re"
	{   Parse(parser, TOK_LOGIC_GT, nullptr, this); continue; }
#line 361 "<stdout>"
yy36:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy37:
	switch (yych) {
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '[':
	case ']':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy36;
	default:	goto yy38;
	}
yy38:
#line 149 "minicalc.re"
	{
        Token * tok = new Token;
        tok->identifier = std::string(token_begin, YYCURSOR);
        Parse(parser, TOK_IDENTIFIER, tok, this);
        continue;
    }
#line 444 "<stdout>"
yy39:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'a':	goto yy54;
	default:	goto yy37;
	}
yy40:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'r':	goto yy55;
	default:	goto yy37;
	}
yy41:
	yych = *++YYCURSOR;
	switch (yych) {
	case '|':	goto yy56;
	default:	goto yy5;
	}
yy42:
	++YYCURSOR;
#line 167 "minicalc.re"
	{   Parse(parser, TOK_LOGIC_NE, nullptr, this); continue; }
#line 467 "<stdout>"
yy44:
	++YYCURSOR;
#line 173 "minicalc.re"
	{   Parse(parser, TOK_LOGIC_AND, nullptr, this); continue; }
#line 472 "<stdout>"
yy46:
	++YYCURSOR;
#line 161 "minicalc.re"
	{   Parse(parser, TOK_POW, nullptr, this); continue; }
#line 477 "<stdout>"
yy48:
	++YYCURSOR;
#line 170 "minicalc.re"
	{   Parse(parser, TOK_LOGIC_LE, nullptr, this); continue; }
#line 482 "<stdout>"
yy50:
	++YYCURSOR;
#line 166 "minicalc.re"
	{   Parse(parser, TOK_LOGIC_EQ, nullptr, this); continue; }
#line 487 "<stdout>"
yy52:
	++YYCURSOR;
#line 171 "minicalc.re"
	{   Parse(parser, TOK_LOGIC_GE, nullptr, this); continue; }
#line 492 "<stdout>"
yy54:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'l':	goto yy58;
	default:	goto yy37;
	}
yy55:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'u':	goto yy59;
	default:	goto yy37;
	}
yy56:
	++YYCURSOR;
#line 174 "minicalc.re"
	{   Parse(parser, TOK_LOGIC_OR, nullptr, this); continue; }
#line 509 "<stdout>"
yy58:
	yych = *++YYCURSOR;
	switch (yych) {
	case 's':	goto yy60;
	default:	goto yy37;
	}
yy59:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'e':	goto yy61;
	default:	goto yy37;
	}
yy60:
	yych = *++YYCURSOR;
	switch (yych) {
	case 'e':	goto yy63;
	default:	goto yy37;
	}
yy61:
	++YYCURSOR;
	switch ((yych = *YYCURSOR)) {
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '[':
	case ']':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy36;
	default:	goto yy62;
	}
yy62:
#line 136 "minicalc.re"
	{   Parse(parser, TOK_TRUE, nullptr, this); continue;}
#line 602 "<stdout>"
yy63:
	++YYCURSOR;
	switch ((yych = *YYCURSOR)) {
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '[':
	case ']':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy36;
	default:	goto yy64;
	}
yy64:
#line 137 "minicalc.re"
	{   Parse(parser, TOK_FALSE, nullptr, this); continue;}
#line 677 "<stdout>"
}
#line 183 "minicalc.re"


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
