
#pragma once

//---------------------------------------------------------------------------
//Эти константы используются для вызова функции sntx_err()
//в случае синтаксической ошибки. При необходимости список
//констант можно расширить.
//ВНИМАНИЕ: константа SYNTAX используется тогда, когда
//интерпритатор не может квалифицировать ошибку.
enum error_msg { 
	SYNTAX,
    UNBAL_PARENS,
    NO_EXP,
    EQUALS_EXPECTED,
    NOT_VAR,
    PARAM_ERR,
    SEMI_EXPECTED,
    UNBAL_BRACES,
    FUNC_UNDEF,
    TYPE_EXPECTED,
    NEST_FUNC,
    RET_NOCALL,
    PAREN_EXPECTED,
    WHILE_EXPECTED,
    QUOTE_EXPECTED,
    NOT_TEMP,
    TOO_MANY_LVARS,
    DIV_BY_ZERO };

//--------------------------------
//Библиотечные функции Little C
int call_getche(void);
int call_putch(void);
int call_puts(void);
int print(void);
int getnum(void);

//--------------------------------
//Функции анализатора
int eval_exp();
int eval_exp0();
int eval_exp1();
int eval_exp2();
int eval_exp3();
int eval_exp4();
int eval_exp5();
int internal_func(char* s);
void sntx_err(int error);
int atom();
//--------------------------------
//char* cto(char*);
