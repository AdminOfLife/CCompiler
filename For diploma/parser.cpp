//---------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//---------------------------------------------------------------------------
#include "globals.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

//---------------------------------------------------------------------------
struct intern_func_type { //��������� ������� �������
    char* f_name; // ��� �������
    int (*p)(); // ��������� �� �������
} intern_func[] = {
    "getche",	call_getche,
    "putch",	call_putch,
    "puts",		call_puts,
    "print",	print,
    "getnum",	getnum,
    //  "exec",	exec,
    "", 0 //���� ������ ������������� �����
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//-------- ����������� ���������� �������������� ���������� Little C --------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// ����� ����� � �������������� ���������� ���������.
void eval_exp(int* value)
{
    get_token();
    if (!*token) {
        sntx_err(NO_EXP);
        return;
    }
    if (*token == ';') {
        *value = 0; // ������ ���������
        return;
    }
    eval_exp0(value);
    putback(); // ������� ��������� ������� �� ������� �����
}
//---------------------------------------------------------------------------
//��������� ��������� � ������������
void eval_exp0(int* value)
{
    char temp[ID_LEN]; // �������� ��� ����������,
    // ������� ������������� ��������
    register int temp_tok;

    if (token_type == lcIDENTIFIER) {
        if (is_var(token)) { // ���� ��� ����������,
            // ����������, ������������� �� �� ��������
            strcpy(temp, token);
            temp_tok = token_type;
            get_token();
            if (*token == '=') { // ��� ������������
                get_token();
                eval_exp0(value); // ��������� ���������� ��������
                assign_var(temp, *value); // ��������� ��������
                return;
            }
            else { // �� ������������
                putback(); // ������������� �������
                strcpy(token, temp);
                token_type = temp_tok;
            }
        }
    }
    eval_exp1(value);
}
//---------------------------------------------------------------------------
// ��������� �������� ���������.
void eval_exp1(int* value)
{
    int partial_value;
    register char op;
    char relops[7] = {
        LT, LE, GT, GE, EQ, NE, 0
    };

    eval_exp2(value);
    op = *token;
    if (strchr(relops, op)) {
        get_token();
        eval_exp2(&partial_value);
        switch (op) { // ���������� ���������� �������� ���������
        case LT:
            *value = *value < partial_value;
            break;
        case LE:
            *value = *value <= partial_value;
            break;
        case GT:
            *value = *value > partial_value;
            break;
        case GE:
            *value = *value >= partial_value;
            break;
        case EQ:
            *value = *value == partial_value;
            break;
        case NE:
            *value = *value != partial_value;
            break;
        }
    }
}
//---------------------------------------------------------------------------
//  ������������ ��� ���������� ���� ������.
void eval_exp2(int* value)
{
    register char op;
    int partial_value;

    eval_exp3(value);
    while ((op = *token) == '+' || op == '-') {
        get_token();
        eval_exp3(&partial_value);
        switch (op) { // ������������ ��� ���������
        case '-':
            *value = *value - partial_value;
            break;
        case '+':
            *value = *value + partial_value;
            break;
        }
    }
}
//---------------------------------------------------------------------------
// ��������� ��� ������� ���� ����������.
void eval_exp3(int* value)
{
    register char op;
    int partial_value, t;

    eval_exp4(value);
    while ((op = *token) == '*' || op == '/' || op == '%') {
        get_token();
        eval_exp4(&partial_value);
        switch (op) { // ���������, ������� ��� ������� �����
        case '*':
            *value = *value * partial_value;
            break;
        case '/':
            if (partial_value == 0)
                sntx_err(DIV_BY_ZERO);
            *value = (*value) / partial_value;
            break;
        case '%':
            t = (*value) / partial_value;
            *value = *value - (t * partial_value);
            break;
        }
    }
}
//---------------------------------------------------------------------------
// ������� + ��� -.
void eval_exp4(int* value)
{
    register char op;

    op = '\0';
    if (*token == '+' || *token == '-') {
        op = *token;
        get_token();
    }
    eval_exp5(value);
    if (op)
        if (op == '-')
            *value = -(*value);
}
//---------------------------------------------------------------------------
// ��������� ��������� � �������.
void eval_exp5(int* value)
{
    if ((*token == '(')) {
        get_token();
        eval_exp0(value); // ���������� ������������
        if (*token != ')')
            sntx_err(PAREN_EXPECTED);
        get_token();
    }
    else
        atom(value);
}
//---------------------------------------------------------------------------
// ��������� �������� �����, ���������� ��� �������.
void atom(int* value)
{
    int i;

    switch (token_type) {
    case lcIDENTIFIER:
        i = internal_func(token);
        if (i != -1) { // ����� ������� �� "����������� ���������"
            *value = (*intern_func[i].p)();
        }
        else if (find_func(token)) { // ����� �������,
            // ������������ �������������
            call();
            *value = ret_value;
        }
        else
            *value = find_var(token); // ��������� �������� ����������
        get_token();
        return;
    case lcNUMBER: // �������� ���������
        *value = atoi(token);
        get_token();
        return;
    case lcDELIMITER: // ��� ���������� ���������?
        if (*token == '\'') {
            *value = *prog;
            prog++;
            if (*prog != '\'')
                sntx_err(QUOTE_EXPECTED);
            prog++;
            get_token();
            return;
        }
        if (*token == ')')
            return; // ��������� ������� ���������
        else
            sntx_err(SYNTAX); // �������������� ������
    default:
        sntx_err(SYNTAX); // �������������� ������
    }
}
//---------------------------------------------------------------------------
// ����� ��������� �� ������.
void sntx_err(int error)
{
    char *p, *temp;
    int linecount = 1;
    register int i;

    static char* e[] = {
        "�������������� ������",
        "������������������ ������",
        "��������� �����������",
        "��������� ���� ���������",
        "�� ����������",
        "������ � ���������",
        "��������� ����� � �������",
        "������������������ �������� ������",
        "������� �� ����������",
        "��������� ������������ ����",
        "������� ����� ��������� ������� �������",
        "�������� return ��� �������",
        "��������� ������",
        "��������� while",
        "��������� ������������� �������",
        "�� ������",
        "������� ����� ��������� ����������",
        "������� �� ����"
    };
    printf("\n%s", cto(e[error]));
    p = p_buf;
    while (p != prog) { //����� ������ ������ � �������
        p++;
        if (*p == '\r') {
            linecount++;
        }
    }
    printf(cto(" �� ����� %d\n"), linecount);

    temp = p;
    for (i = 0; i < 20 && p > p_buf && *p != '\n'; i++, p--)
        ;
    for (i = 0; i < 30 && p <= temp; i++, p++)
        printf("%c", *p);

    longjmp(e_buf, 1); // ������� � ���������� �����
}

//---------------------------------------------------------------------------
// ���������� ����� ������� �� ���������� ����������, ��� -1, ���� �� �������.
int internal_func(char* s)
{
    int i;

    for (i = 0; intern_func[i].f_name[0]; i++) {
        if (!strcmp(intern_func[i].f_name, s))
            return i;
    }
    return -1;
}
//---------------------------------------------------------------------------
// ���������� true (������), ���� � - �����������.
int isdelim(char c)
{
    if (strchr(" !;,+-<>'/*%^=()", c) || c == 9 || c == '\r' || c == 0)
        return 1;
    return 0;
}
//---------------------------------------------------------------------------
// ���������� 1, ���� � - ������ ��� ���������.
int iswhite(char c)
{
    if (c == ' ' || c == '\t')
        return 1;
    else
        return 0;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//-------------------- ������������ ������� Little C ------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// ���������� ������� � �������. ���� ���������� �� ������������
//  _getche(), �� ������� ����������� getchar()
int call_getche()
{
    char ch;
    ch = getchar();
    while (*prog != ')')
        prog++;
    prog++; // ����������� � ����� ������
    return ch;
}
//---------------------------------------------------------------------------
// ����� ������� �� �����.
int call_putch()
{
    int value;

    eval_exp(&value);
    printf("%c", value);
    return value;
}
//---------------------------------------------------------------------------
// ����� ������� puts().
int call_puts(void)
{
    get_token();
    if (*token != '(')
        sntx_err(PAREN_EXPECTED);
    get_token();
    if (token_type != lcSTRING)
        sntx_err(QUOTE_EXPECTED);
    puts(token);
    get_token();
    if (*token != ')')
        sntx_err(PAREN_EXPECTED);

    get_token();
    if (*token != ';')
        sntx_err(SEMI_EXPECTED);
    putback();
    return 0;
}

//---------------------------------------------------------------------------
// ����� ������������ �����
//
//int exec(

//---------------------------------------------------------------------------
// ���������� ������ ����� � ����������.
int getnum(void)
{
    char s[80];

    gets(s);
    while (*prog != ')')
        prog++;
    prog++; // ����������� � ����� ������
    return atoi(s);
}
//---------------------------------------------------------------------------
// ���������� ������� ����������� ������.
int print(void)
{
    int i;

    get_token();
    if (*token != '(')
        sntx_err(PAREN_EXPECTED);

    get_token();
    if (token_type == lcSTRING) { // ����� ������
        // char *p;// = cto(token);
        //    OemToAnsi(token,p);
        printf("%s ", token);
    }
    else { // ����� �����
        putback();
        eval_exp(&i);
        printf("%d ", i);
    }

    get_token();

    if (*token != ')')
        sntx_err(PAREN_EXPECTED);

    get_token();
    if (*token != ';')
        sntx_err(SEMI_EXPECTED);
    putback();
    return 0;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//��� ����������� ������� ����
char* cto(char* s)
{
    //char *c;
    //CharToOem(s,c);
    return s;
}
//---------------------------------------------------------------------------
