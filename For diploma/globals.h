#pragma once
//---------------------------------------------------------------------------
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
//---------------------------------------------------------------------------
extern char* p_buf; //��������� �� ������ ������ ���������
extern jmp_buf e_buf; //�������� ���������� ��� longjmp()
extern char* prog; //������� ������� � �������� ������ ���������

//���������
extern int ret_value; // ������������ �������� ������� func_ret atom
extern int lvartos; //������ � ����� ��������� ���������� main call is_var find_var assign_var local_push get_params
extern char token[80]; //��������� ������������� ������� main print call_puts putback get_token atom eval_exp5 eval_exp4 eval_exp3 eval_exp2 eval_exp1 eval_exp0 eval_exp exec_for find_eob is_var find_var get_params get_args call decl_local decl_global interp_block prescan
extern char token_type; //�������� ��� ������� prescan print call_puts get_token atom eval_exp0 interp_block get_params
extern char tok; //���������� ������������� ������� prescan get_token exec_do exec_if get_params decl_local decl_global
// interp_block
#define cto(a) a

#define TRUE
#define FALSE

