#pragma once

//---------------------------------------------------------------------------
#define PROG_SIZE 10000
#define ID_LEN 31
#define NUM_FUNC 100
#define NUM_GLOBAL_VARS 100
#define NUM_PARAMS 31
#define NUM_LOCAL_VARS 200

//---------------------------------------------------------------------------
//Функции интерпритатора
int interpreter_init(char* fname);
int load_program(char* p, char* fname);
void prescan(void);
void interp_block(void);
char* find_func(const char* name);
void decl_global(void);
void decl_local(void);
void call(void);
void get_args(void);
void get_params(void);
void func_ret(void);
void local_push(struct var_type i);
int func_pop(void);
void func_push(int i);
void assign_var(char* var_name, int value);
int find_var(char* s);
int is_var(char* s);
void exec_if(void);
void exec_while(void);
void exec_do(void);
void find_eob(void);
void exec_for(void);
