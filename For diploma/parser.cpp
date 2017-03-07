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
struct intern_func_type { //внутреняя таблица функций
    char* f_name; // имя функции
    int (*p)(); // указатель на функцию
} intern_func[] = {
    "getche",	call_getche,
    "putch",	call_putch,
    "puts",		call_puts,
    "print",	print,
    "getnum",	getnum,
    //  "exec",	exec,
    "", 0 //этот список заканчивается нулем
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//-------- Рекурсивный нисходящий синтаксический анализатор Little C --------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Точка входа в синтаксический анализатор выражений.
void eval_exp(int* value)
{
    get_token();
    if (!*token) {
        sntx_err(NO_EXP);
        return;
    }
    if (*token == ';') {
        *value = 0; // пустое выражение
        return;
    }
    eval_exp0(value);
    putback(); // возврат последней лексемы во входной поток
}
//---------------------------------------------------------------------------
//Обработка выражения в присваивании
void eval_exp0(int* value)
{
    char temp[ID_LEN]; // содержит имя переменной,
    // которой присваивается значение
    register int temp_tok;

    if (token_type == lcIDENTIFIER) {
        if (is_var(token)) { // если эта переменная,
            // посмотреть, присваивается ли ей значение
            strcpy(temp, token);
            temp_tok = token_type;
            get_token();
            if (*token == '=') { // это присваивание
                get_token();
                eval_exp0(value); // вычислить присваемое значение
                assign_var(temp, *value); // присвоить значение
                return;
            }
            else { // не присваивание
                putback(); // востановление лексемы
                strcpy(token, temp);
                token_type = temp_tok;
            }
        }
    }
    eval_exp1(value);
}
//---------------------------------------------------------------------------
// Обработка операций сравнения.
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
        switch (op) { // вычисление результата операции сравнения
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
//  Суммирование или вычисление двух термов.
void eval_exp2(int* value)
{
    register char op;
    int partial_value;

    eval_exp3(value);
    while ((op = *token) == '+' || op == '-') {
        get_token();
        eval_exp3(&partial_value);
        switch (op) { // суммирование или вычитание
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
// Умножение или деление двух множителей.
void eval_exp3(int* value)
{
    register char op;
    int partial_value, t;

    eval_exp4(value);
    while ((op = *token) == '*' || op == '/' || op == '%') {
        get_token();
        eval_exp4(&partial_value);
        switch (op) { // умножение, деление или деление целых
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
// Унарный + или -.
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
// Обработка выражения в скобках.
void eval_exp5(int* value)
{
    if ((*token == '(')) {
        get_token();
        eval_exp0(value); // вычисление подвыражения
        if (*token != ')')
            sntx_err(PAREN_EXPECTED);
        get_token();
    }
    else
        atom(value);
}
//---------------------------------------------------------------------------
// Получение значения числа, переменной или функции.
void atom(int* value)
{
    int i;

    switch (token_type) {
    case lcIDENTIFIER:
        i = internal_func(token);
        if (i != -1) { // вызов функции из "стандартной билиотеки"
            *value = (*intern_func[i].p)();
        }
        else if (find_func(token)) { // вызов функции,
            // определенной пользователем
            call();
            *value = ret_value;
        }
        else
            *value = find_var(token); // получение значения переменной
        get_token();
        return;
    case lcNUMBER: // числовая константа
        *value = atoi(token);
        get_token();
        return;
    case lcDELIMITER: // это символьная константа?
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
            return; // обработка пустого выражения
        else
            sntx_err(SYNTAX); // синтаксическая ошибка
    default:
        sntx_err(SYNTAX); // синтаксическая ошибка
    }
}
//---------------------------------------------------------------------------
// Вывод сообщения об ошибке.
void sntx_err(int error)
{
    char *p, *temp;
    int linecount = 1;
    register int i;

    static char* e[] = {
        "синтаксическая ошибка",
        "несбалансированные скобки",
        "выражение отсутствует",
        "ожидается знак равенства",
        "не переменная",
        "ошибка в параметре",
        "ожидается точка с запятой",
        "несбалансированные фигурные скобки",
        "функция не определена",
        "ожидается спецификатор типа",
        "слишком много вложенных вызовов функций",
        "оператор return вне функции",
        "ожидаются скобки",
        "ожидается while",
        "ожидается закрывающаяся кавычка",
        "не строка",
        "слишком много локальных переменных",
        "деление на нуль"
    };
    printf("\n%s", cto(e[error]));
    p = p_buf;
    while (p != prog) { //поиск номера строки с ошибкой
        p++;
        if (*p == '\r') {
            linecount++;
        }
    }
    printf(cto(" на линии %d\n"), linecount);

    temp = p;
    for (i = 0; i < 20 && p > p_buf && *p != '\n'; i++, p--)
        ;
    for (i = 0; i < 30 && p <= temp; i++, p++)
        printf("%c", *p);

    longjmp(e_buf, 1); // возврат в безопасную точку
}

//---------------------------------------------------------------------------
// Возвращает идекс функции во внутренней библиотеке, или -1, если не найдена.
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
// Возвращает true (ИСТИНА), если с - разделитель.
int isdelim(char c)
{
    if (strchr(" !;,+-<>'/*%^=()", c) || c == 9 || c == '\r' || c == 0)
        return 1;
    return 0;
}
//---------------------------------------------------------------------------
// Возвращает 1, если с - пробел или табуляция.
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
//-------------------- Библиотечные функции Little C ------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Считывание символа с консоли. Если компилятор не поддерживает
//  _getche(), то следует использвать getchar()
int call_getche()
{
    char ch;
    ch = getchar();
    while (*prog != ')')
        prog++;
    prog++; // продвижение к концу строки
    return ch;
}
//---------------------------------------------------------------------------
// Вывод символа на экран.
int call_putch()
{
    int value;

    eval_exp(&value);
    printf("%c", value);
    return value;
}
//---------------------------------------------------------------------------
// Вызов функции puts().
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
// Вызов исполняемого файла
//
//int exec(

//---------------------------------------------------------------------------
// Считывание целого числа с клавиатуры.
int getnum(void)
{
    char s[80];

    gets(s);
    while (*prog != ')')
        prog++;
    prog++; // продвижение к концу строки
    return atoi(s);
}
//---------------------------------------------------------------------------
// Встроенная функция консольного вывода.
int print(void)
{
    int i;

    get_token();
    if (*token != '(')
        sntx_err(PAREN_EXPECTED);

    get_token();
    if (token_type == lcSTRING) { // вывод строки
        // char *p;// = cto(token);
        //    OemToAnsi(token,p);
        printf("%s ", token);
    }
    else { // вывод числа
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
//Для отображения русских букв
char* cto(char* s)
{
    //char *c;
    //CharToOem(s,c);
    return s;
}
//---------------------------------------------------------------------------
