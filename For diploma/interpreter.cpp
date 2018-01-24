//---------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <conio.h>
//---------------------------------------------------------------------------
#include "globals.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"



//---------------------------------------------------------------------------
//�������������
int gvar_index; //������ � ������� ���������� ���������� main decl_global
int func_index; //������ � ������� ������� prescan *find_func
int call_stack[NUM_FUNC]; //func_pop is_var find_var func_push assign_var
int functos; //������ ������� ����� ������ ������� main is_var find_var assign_var func_push func_pop
//---------------------------------------------------------------------------
struct func_type { //��� ���� ������ �������.
    char func_name[ID_LEN];
    int ret_type;
    char* loc; /* ����� ����� ����� � ���� */
} func_table[NUM_FUNC];
// ������ ���� ��������� �������� ���������� � ���������� ����������.
struct var_type {
    char var_name[32];
    int v_type;
    union {
		double value;
	};
} global_vars[NUM_GLOBAL_VARS];
struct var_type local_var_stack[NUM_LOCAL_VARS];

MODULE_IDENT( "@(#)interpreter.cpp");

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------- ������������� Little C -----------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//������������� ��������������
int interpreter_init(char* fname)
{
    //��������� ������ ��� ���������
    if ((p_buf = (char*)malloc(PROG_SIZE)) == NULL) {
        //�������� ������ �� �������
        printf(cto("�������� ������ �� �������"));
        getch();
        exit(1);
    }

    // �������� ��������� ��� ����������
    if (!load_program(p_buf, fname)) {
        getch();
        exit(1);
    }
    if (setjmp(e_buf)) // ������������� ������ long jump
    {
        getch();
        exit(1);
    }

    gvar_index = 0; //������������� ������� ���������� ����������

    //��������� ��������� ��������� �� ������ ������ ���������
    prog = p_buf;
    prescan(); // ����������� ������� ���� �������
    // � ���������� ���������� ���������

    lvartos = 0; // ������������� ������� ����� ��������� ����������
    functos = 0; // ������������� ������� ����� ������ (CALL)

    // ������ ���������� main()
    prog = find_func("main"); // ����� ����� ����� ���������

    if (!prog) { // ������� main() ����������� ��� �����������
        // main() �� �������
        printf(cto("main() �� �������.\n"));
        getch();
        exit(1);
    }

    prog--; // ������� � ������������� ������ (
    strcpy(token, "main");
    return 0;
}

// �������� ���������.
int load_program(char* p, char* fname)
{
    FILE* fp;
    int i = 0;

    if ((fp = fopen(fname, "rb")) == NULL)
        return 0;

    i = 0;
    do {
        *p = getc(fp);
        p++;
        i++;
    } while (!feof(fp) && i < PROG_SIZE);

    if (*(p - 2) == 0x1a)
        *(p - 2) = '\0'; //��������� ���������
    //������� ��������
    else
        *(p - 1) = '\0';
    fclose(fp);
    return 1;
}
//---------------------------------------------------------------------------
//����� ������ ���� ������� � ��������� � ��������� ���������� ����������.
void prescan(void)
{
	FCT_IDENT();
    char *p, *tp;
    char temp[32];
    int datatype;
    int brace = 0; // ���� brace = 0, �� �������
    // ������� ��������� ��������� ���������
    // � �� �����-���� �������.
    //TODO: DO SOMETHING
    p = prog;
    func_index = 0;
    do {
        while (brace) { //����� ���� �������
            get_token();
            if (*token == '{')
                brace++;
            if (*token == '}')
                brace--;
        }

        tp = prog; //����������� ������� �������
        get_token();
        // ��� ���������� ���������� ��� ������������� �������� �������
        if (tok == lcCHAR || tok == lcINT) {
            datatype = tok; // ����������� ���� ������
            get_token();
            if (token_type == lcIDENTIFIER) {
                strcpy(temp, token);
                get_token();
                if (*token != '(') { // ��� ������ ���� ���������� ����������
                    prog = tp; // ������� � ������ ����������
                    decl_global();
                }
                else if (*token == '(') { // ��� ������ ���� �������
                    func_table[func_index].loc = prog;
                    func_table[func_index].ret_type = datatype;
                    strcpy(func_table[func_index].func_name, temp);
                    func_index++;
                    while (*prog != ')')
                        prog++;
                    prog++;
                    //������ prog ��������� �� �������������
                    //�������� ������ �������
                }
                else
                    putback();
            }
        }
        else if (*token == '{')
            brace++;
    } while (tok != lcFINISHED);
    prog = p;
	END_FCT();
}
//---------------------------------------------------------------------------
// ������������� ������ ��������� ��� �����. ����� interp_block() ����������
// ���������� ����� ������� ������, � main() ����������� ��������� �������������
// �������� ������ ��� �������� return.
void interp_block(void)
{
    int value;
    char block = 0;

    do {
        token_type = get_token();

        // ��� ������������� ������ ����������������
        //  ����� ������ ����� � �������.

        // ����������� ���� �������
        if (token_type == lcIDENTIFIER) {
            // ��� �� ������������������ �����,
            // �������������� ���������.
            putback(); // ������� ��������� ������� �����
            //  ��� ���������� ��������� �������� eval_exp()
            eval_exp(&value); // ��������� ���������
            if (*token != ';')
                sntx_err(SEMI_EXPECTED);
        }
        else if (token_type == lcBLOCK) {
            // ���� ��� ����������� �����
            if (*token == '{') // ����
                block = 1; // ������������� �����, � �� ���������
            else
                return; // ��� }, �������
        }
        else // ����������������� �����
            switch (tok) {
            case lcCHAR:
			case lcSTRUCT:
            case lcINT: // ���������� ��������� ����������
                putback();
                decl_local();
                break;
            case lcRETURN: // ������� �� ������ �������
                func_ret();
                return;
            case lcIF: // ��������� ��������� if
                exec_if();
                break;
            case lcELSE: // ��������� ��������� else
                find_eob(); // ����� ����� ����� else
                // � ����������� ����������
                break;
            case lcWHILE: // ��������� ����� while
                exec_while();
                break;
            case lcDO: // ��������� ����� do-while
                exec_do();
                break;
            case lcFOR: // ��������� ����� for
                exec_for();
                break;
            case lcEND:
                exit(0);
            }
    } while (tok != lcFINISHED && block);
}
//---------------------------------------------------------------------------
// ������� ������ ����� ����� ������ �������. ������� NULL, ���� �� ������.
char* find_func(const char* name)
{
    register int i;

    for (i = 0; i < func_index; i++)
        if (!strcmp(name, func_table[i].func_name))
            return func_table[i].loc;

    return NULL;
}
//---------------------------------------------------------------------------
// ���������� ���������� ����������.
void decl_global(void)
{
	FCT_IDENT();
    int vartype;

    get_token(); // ����������� ����

    vartype = tok; // ����������� ���� ����������

    do { // ��������� ������
        global_vars[gvar_index].v_type = vartype;
        global_vars[gvar_index].value = 0; // ������������� �����
        get_token(); // ����������� �����
        strcpy(global_vars[gvar_index].var_name, token);
        get_token();
        gvar_index++;
    } while (*token == ',');
    if (*token != ';')
        sntx_err(SEMI_EXPECTED);
	END_FCT();
}
//---------------------------------------------------------------------------
// ���������� ��������� ����������.
void decl_local(void)
{
	FCT_IDENT();
    struct var_type i;
	
	if (tok == lcSTRUCT)
	{
		get_token();
		if (get_token() == lcIDENTIFIER)
		{
			get_token();
			if (*token == '{')
			{
				find_eob();
				//decl_local();
			}
			else
			{
				sntx_err(PAREN_EXPECTED);
			}
		}
		else
		{
			sntx_err(SYNTAX);
		}
		END_FCT();
	}
	else
	{
		get_token(); // ����������� ����

	    i.v_type = tok;
	    i.value = 0; // ������������� �����
	
	    do { // ��������� ������
	        get_token(); // ����������� ���� ���������
	        strcpy(i.var_name, token);
	        local_push(i);
	        get_token();
	    } while (*token == ',');
	    if (*token != ';')
	        sntx_err(SEMI_EXPECTED);
	    END_FCT();	
	}   
}
//---------------------------------------------------------------------------
// ����� �������.
void call(void)
{
	FCT_IDENT();
    char *loc, *temp;
    int lvartemp;

    loc = find_func(token); // ����� ����� ����� �������
    if (loc == NULL)
        sntx_err(FUNC_UNDEF); // ������� �� ����������
    else {
        lvartemp = lvartos; // ����������� ������� �����
        //  ��������� ����������
        get_args(); // ��������� ��������� �������
        temp = prog; // ����������� ������ ��������
        func_push(lvartemp); // ����������� ������� �����
        //  ��������� ����������
        prog = loc; // ������������� prog � ������ �������
        get_params(); // �������� ���������� �������
        // ���������� ����������
        interp_block(); // ������������� �������
        prog = temp; // �������������� prog
        lvartos = func_pop(); // �������������� �����
        // ��������� ����������
    }
	END_FCT();
}
//---------------------------------------------------------------------------
// ������������ ���������� ������� � ���� ��������� ����������.
void get_args(void)
{
    int value, count, temp[NUM_PARAMS];
    struct var_type i;

    count = 0;
    get_token();
    if (*token != '(')
        sntx_err(PAREN_EXPECTED);

    // ��������� ������ ��������
    do {
        eval_exp(&value);
        temp[count] = value; // ��������� �����������
        get_token();
        count++;
    } while (*token == ',');
    count--;
    // ���������� � local_var_stack � �������� �������
    for (; count >= 0; count--) {
        i.value = temp[count];
        i.v_type = lcARG;
        local_push(i);
    }
}
//---------------------------------------------------------------------------
// ��������� ���������� �������.
void get_params(void)
{
    struct var_type* p;
    int i;

    i = lvartos - 1;
    do { // ��������� ������ ����������
        get_token();
        p = &local_var_stack[i];
        if (*token != ')') {
            if (tok != lcINT && tok != lcCHAR)
                sntx_err(TYPE_EXPECTED);

            p->v_type = token_type;
            get_token();

            // ���������� ����� ���������� � ����������,
            //  ��� ����������� � ����� ��������� ����������
            strcpy(p->var_name, token);
            get_token();
            i--;
        }
        else
            break;
    } while (*token == ',');
    if (*token != ')')
        sntx_err(PAREN_EXPECTED);
}
//---------------------------------------------------------------------------
// ������� �� �������.
void func_ret(void)
{
    int value;

    value = 0;
    // ��������� ������������� ��������, ���� ��� ����
    eval_exp(&value);

    ret_value = value;
}
//---------------------------------------------------------------------------
// ���������� ��������� ����������.
void local_push(struct var_type i)
{
    if (lvartos > NUM_LOCAL_VARS)
        sntx_err(TOO_MANY_LVARS);

    local_var_stack[lvartos] = i;
    lvartos++;
}
//---------------------------------------------------------------------------
// ������������ ������� � ����� ��������� ����������.
int func_pop(void)
{
    functos--;
    if (functos < 0)
        sntx_err(RET_NOCALL);
    return call_stack[functos];
}
//---------------------------------------------------------------------------
// ������ ������� � ���� ��������� ����������.
void func_push(int i)
{
    if (functos > NUM_FUNC)
        sntx_err(NEST_FUNC);
    call_stack[functos] = i;
    functos++;
}
//---------------------------------------------------------------------------
// ������������ ���������� ��������.
void assign_var(char* var_name, int value)
{
    register int i;

    //�������� ������� ��������� ����������
    for (i = lvartos - 1; i >= call_stack[functos - 1]; i--) {
        if (!strcmp(local_var_stack[i].var_name, var_name)) {
            local_var_stack[i].value = value;
            return;
        }
    }
    if (i < call_stack[functos - 1])
        // ���� ���������� �����������,
        // ���� �� � ������� ���������� ����������
        for (i = 0; i < NUM_GLOBAL_VARS; i++)
            if (!strcmp(global_vars[i].var_name, var_name)) {
                global_vars[i].value = value;
                return;
            }
    sntx_err(NOT_VAR); //���������� �� �������
}
//---------------------------------------------------------------------------
// ��������� �������� ����������.
int find_var(char* s)
{
    register int i;

    // �������� ������� ����������
    for (i = lvartos - 1; i >= call_stack[functos - 1]; i--)
        if (!strcmp(local_var_stack[i].var_name, token))
            return local_var_stack[i].value;

    // � ��������� ������ ��������,
    // ����� ���� ��� ���������� ����������
    for (i = 0; i < NUM_GLOBAL_VARS; i++)
        if (!strcmp(global_vars[i].var_name, s))
            return global_vars[i].value;

    sntx_err(NOT_VAR); // ���������� �� �������
    return -1;
}
//---------------------------------------------------------------------------
// ���� �������������� �������� ����������, �� ������������ 1, ����� 0.
int is_var(char* s)
{
    register int i;

    // ��� ��������� ���������� ?
    for (i = lvartos - 1; i >= call_stack[functos - 1]; i--)
        if (!strcmp(local_var_stack[i].var_name, token))
            return 1;

    // ���� ��� - ����� ����� ���������� ����������
    for (i = 0; i < NUM_GLOBAL_VARS; i++)
        if (!strcmp(global_vars[i].var_name, s))
            return 1;

    return 0;
}
//---------------------------------------------------------------------------
// ���������� ��������� if.
void exec_if(void)
{
    int cond;

    eval_exp(&cond); // ���������� if-���������

    if (cond) { // ������ - ������������� if-�����������
        interp_block();
    }
    else { // � ��������� ������ ������� if-�����������
        // � ���������� else-�����������, ���� ��� ����
        find_eob(); // ����� ����� �����
        get_token();

        if (tok != lcELSE) {
            putback(); // �������������� �������,
            // ���� else-����������� ����������
            return;
        }
        interp_block();
    }
}
//---------------------------------------------------------------------------
// ���������� ����� while.
void exec_while(void)
{
    int cond;
    char* temp;

    putback();
    temp = prog; // ����������� ������ ������ ����� while
    get_token();
    eval_exp(&cond); // ���������� ������������ ���������
    if (cond)
        interp_block(); // ���� ��� �������, �� ���������
    // �������������
    else { // � ��������� ������ ���� ������������
        find_eob();
        return;
    }
    prog = temp; // ������� � ������ �����
}
//---------------------------------------------------------------------------
// ���������� ����� do.
void exec_do(void)
{
    int cond;
    char* temp;

    putback();
    temp = prog; // ����������� ������ ������ �����

    get_token(); // ����� ������ �����
    interp_block(); // ������������� �����
    get_token();
    if (tok != lcWHILE)
        sntx_err(WHILE_EXPECTED);
    eval_exp(&cond); // �������� ������� �����
    if (cond)
        prog = temp; // ���� ������� �������,
    //�� ���� �����������, � ��������� ������ ����������
    //����� �� �����
}
//---------------------------------------------------------------------------
// ����� ����� �����.
void find_eob(void)
{
	FCT_IDENT();
    int brace;

    get_token();
    brace = 1;
    do {
        get_token();
        if (*token == '{')
            brace++;
        else if (*token == '}')
            brace--;
    } while (brace);
    END_FCT();
}
//---------------------------------------------------------------------------
// ���������� ����� for.
void exec_for(void)
{
    int cond;
    char *temp, *temp2;
    int brace;

    get_token();
    eval_exp(&cond); // ���������������� ���������
    if (*token != ';')
        sntx_err(SEMI_EXPECTED);
    prog++; // ������� ; */
    temp = prog;
    for (;;) {
        eval_exp(&cond); // �������� �������
        if (*token != ';')
            sntx_err(SEMI_EXPECTED);
        prog++; // ������� ;
        temp2 = prog;

        // ����� ������ ���� �����
        brace = 1;
        while (brace) {
            get_token();
            if (*token == '(')
                brace++;
            if (*token == ')')
                brace--;
        }

        if (cond)
            interp_block(); // ���� ������� ���������,
        // �� ��������� �������������
        else { // � ��������� ������ ������ ����
            find_eob();
            return;
        }
        prog = temp2;
        eval_exp(&cond); // ��������� ����������
        prog = temp; // ������� � ������ �����
    }
}
