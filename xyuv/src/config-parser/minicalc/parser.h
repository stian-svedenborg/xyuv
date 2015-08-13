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

/* This file was automatically generated.  Do not edit! */
#define ParseTOKENTYPE MiniCalc::Token*
#define ParseARG_PDECL , MiniCalc *owner 
void Parse(void *yyp,int yymajor,ParseTOKENTYPE yyminor ParseARG_PDECL);
#if defined(YYTRACKMAXSTACKDEPTH)
int ParseStackPeak(void *p);
#endif
void ParseFree(void *p,void(*freeProc)(void *));
void *ParseAlloc(void *(*mallocProc)(size_t));
#if !defined(NDEBUG)
void ParseTrace(FILE *TraceFILE,char *zTracePrompt);
#endif
#define ParseARG_STORE yypParser->owner  = owner 
#define ParseARG_FETCH  MiniCalc *owner  = yypParser->owner 
#define ParseARG_SDECL  MiniCalc *owner ;
#define TOK_RPAREN                         12
#define TOK_COMMA                          11
#define TOK_LPAREN                         10
#define TOK_IDENTIFIER                      9
#define TOK_INT                             8
#define TOK_POW                             7
#define TOK_UNARY_MINUS                     6
#define TOK_MOD                             5
#define TOK_MUL                             4
#define TOK_DIV                             3
#define TOK_MINUS                           2
#define TOK_PLUS                            1
#define INTERFACE 0
