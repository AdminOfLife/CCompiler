
#pragma once

//---------------------------------------------------------------------------
//��� ��������� ������������ ��� ������ ������� sntx_err()
//� ������ �������������� ������. ��� ������������� ������
//�������� ����� ���������.
//��������: ��������� SYNTAX ������������ �����, �����
//������������� �� ����� ��������������� ������.
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
//������������ ������� Little C
int call_getche(void);
int call_putch(void);
int call_puts(void);
int print(void);
int getnum(void);

//--------------------------------
//������� �����������
void eval_exp(int* value);
void eval_exp0(int* value);
void eval_exp1(int* value);
void eval_exp2(int* value);
void eval_exp3(int* value);
void eval_exp4(int* value);
void eval_exp5(int* value);
int internal_func(char* s);
void sntx_err(int error);
void atom(int* value);
//--------------------------------
//char* cto(char*);
