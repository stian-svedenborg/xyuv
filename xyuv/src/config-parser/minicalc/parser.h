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
#define TOK_BOOL                           27
#define TOK_IF                             26
#define TOK_FALSE                          25
#define TOK_TRUE                           24
#define TOK_RPAREN                         23
#define TOK_COMMA                          22
#define TOK_LPAREN                         21
#define TOK_IDENTIFIER                     20
#define TOK_INT                            19
#define TOK_RPARENS                        18
#define TOK_LPARENS                        17
#define TOK_POW                            16
#define TOK_LOGIC_NEG                      15
#define TOK_UNARY_MINUS                    14
#define TOK_MOD                            13
#define TOK_MUL                            12
#define TOK_DIV                            11
#define TOK_MINUS                          10
#define TOK_PLUS                            9
#define TOK_LOGIC_GE                        8
#define TOK_LOGIC_LE                        7
#define TOK_LOGIC_GT                        6
#define TOK_LOGIC_LT                        5
#define TOK_LOGIC_NE                        4
#define TOK_LOGIC_EQ                        3
#define TOK_LOGIC_OR                        2
#define TOK_LOGIC_AND                       1
#define INTERFACE 0
