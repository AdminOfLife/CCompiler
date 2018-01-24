#undef UNICODE
// //---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma argsused
//---------------------------------------------------------------------------
#include <stdio.h> //printf()
#include <setjmp.h> //setjmp()
#include <string.h> //strcpy()
#include <windows.h> //CharToOem()
#include <ctype.h> //isdigit()
#include <stdlib.h> //exit()
#include <conio.h> //getch()
#include <locale.h>
//#include <math.h>   //?

//#include "lexer.h"
#include "interpreter.h"
// ---------------------------------------------------------------------------
// #define PROG_SIZE       10000
// #define ID_LEN          31
// #define NUM_FUNC        100
// #define NUM_GLOBAL_VARS 100
// #define NUM_PARAMS      31
// #define NUM_LOCAL_VARS  200
//
//#define NUM_BLOCK       100//?
//#define FUNC_CALLS      31//?
//#define LOOP_NEST       31//?
//#define FOR_NEST        31//?
#define cto(a) a
//---------------------------------------------------------------------------
char* p_buf; //��������� �� ������ ������ ���������
jmp_buf e_buf; //�������� ���������� ��� longjmp()
char* prog; //������� ������� � �������� ������ ���������

//���������
int ret_value; // ������������ �������� ������� func_ret atom
int lvartos; //������ � ����� ��������� ���������� main call is_var find_var assign_var local_push get_params
char token[80]; //��������� ������������� ������� main print call_puts putback get_token atom eval_exp5 eval_exp4 eval_exp3 eval_exp2 eval_exp1 eval_exp0 eval_exp exec_for find_eob is_var find_var get_params get_args call decl_local decl_global interp_block prescan
char token_type; //�������� ��� ������� prescan print call_puts get_token atom eval_exp0 interp_block get_params
char tok; //���������� ������������� ������� prescan get_token exec_do exec_if get_params decl_local decl_global interp_block

//--------------------------------
//char* cto(char*);
//--------------------------------
//������������ ������� Little C
int call_getche(void);
int call_putch(void);
int call_puts(void);
int print(void);
int getnum(void);
//---------------------------------------------------------------------------
int main(int argc, char* argv[])
{ // ;
	setlocale(LC_ALL, "rus");
#if defined(_DEBUG) || defined(__DEBUG__)
	char* filename = "test2.c";
	argc = 2;
	argv[1] = filename;
#endif // _DEBUG
	if (argc != 2) {
		//����������: littlec <���_�����>
		printf(cto("����������: littlec <���_�����>\n"));
		//getch();
		exit(1);
	}

	interpreter_init(argv[1]);
	call(); // ������ ������������� main()
	getch();
	return 0;
}
