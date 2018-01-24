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
char* p_buf; //указывает на начало буфера программы
jmp_buf e_buf; //содержит информацию для longjmp()
char* prog; //текущая позиция в исходном тексте программы

//глобально
int ret_value; // возвращаемое значение функции func_ret atom
int lvartos; //индекс в стеке локальных переменных main call is_var find_var assign_var local_push get_params
char token[80]; //строковое представление лексемы main print call_puts putback get_token atom eval_exp5 eval_exp4 eval_exp3 eval_exp2 eval_exp1 eval_exp0 eval_exp exec_for find_eob is_var find_var get_params get_args call decl_local decl_global interp_block prescan
char token_type; //содержит тип лексемы prescan print call_puts get_token atom eval_exp0 interp_block get_params
char tok; //внутреннее представление лексемы prescan get_token exec_do exec_if get_params decl_local decl_global interp_block

//--------------------------------
//char* cto(char*);
//--------------------------------
//Библиотечные функции Little C
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
		//Применение: littlec <имя_файла>
		printf(cto("Применение: littlec <имя_файла>\n"));
		//getch();
		exit(1);
	}

	interpreter_init(argv[1]);
	call(); // начало интерпритации main()
	getch();
	return 0;
}
