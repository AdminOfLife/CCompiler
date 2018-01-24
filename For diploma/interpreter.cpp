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
//интерпритатор
int gvar_index; //индекс в таблице глобальных переменных main decl_global
int func_index; //индекс в таблице функций prescan *find_func
int call_stack[NUM_FUNC]; //func_pop is_var find_var func_push assign_var
int functos; //индекс вершины стека вызова функции main is_var find_var assign_var func_push func_pop
//---------------------------------------------------------------------------
struct func_type { //Это стек вызова функции.
    char func_name[ID_LEN];
    int ret_type;
    char* loc; /* адрес точки входа в файл */
} func_table[NUM_FUNC];
// Массив этой структуры содержит информацию о глобальный переменных.
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
//---------------------- Интерпретатор Little C -----------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//Инициализация интерпретатора
int interpreter_init(char* fname)
{
    //выделение памяти для программы
    if ((p_buf = (char*)malloc(PROG_SIZE)) == NULL) {
        //Выделить память не удалось
        printf(cto("Выделить память не удалось"));
        getch();
        exit(1);
    }

    // загрузка программы для выполнения
    if (!load_program(p_buf, fname)) {
        getch();
        exit(1);
    }
    if (setjmp(e_buf)) // инициализация буфера long jump
    {
        getch();
        exit(1);
    }

    gvar_index = 0; //инициализация индекса глобальных переменных

    //установка указателя программы на начало буфера программы
    prog = p_buf;
    prescan(); // определение адресов всех функций
    // и глобальных переменных программы

    lvartos = 0; // инициализация индекса стека локальных переменных
    functos = 0; // инициализация индекса стека вызова (CALL)

    // первой вызывается main()
    prog = find_func("main"); // поиск точки входа программы

    if (!prog) { // функция main() неправильна или отсутствует
        // main() не найдена
        printf(cto("main() не найдена.\n"));
        getch();
        exit(1);
    }

    prog--; // возврат к открывающейся скобке (
    strcpy(token, "main");
    return 0;
}

// Загрузка программы.
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
        *(p - 2) = '\0'; //программа кончается
    //нулевым символом
    else
        *(p - 1) = '\0';
    fclose(fp);
    return 1;
}
//---------------------------------------------------------------------------
//Найти адреса всех функций в программе и запомнить глобальные переменные.
void prescan(void)
{
	FCT_IDENT();
    char *p, *tp;
    char temp[32];
    int datatype;
    int brace = 0; // Если brace = 0, то текущая
    // позиция указателя программы находится
    // в не какой-либо функции.
    //TODO: DO SOMETHING
    p = prog;
    func_index = 0;
    do {
        while (brace) { //обход кода функции
            get_token();
            if (*token == '{')
                brace++;
            if (*token == '}')
                brace--;
        }

        tp = prog; //запоминание текущей позиции
        get_token();
        // тип глобальной переменной или возвращаемого значения функции
        if (tok == lcCHAR || tok == lcINT) {
            datatype = tok; // запоминание типа данных
            get_token();
            if (token_type == lcIDENTIFIER) {
                strcpy(temp, token);
                get_token();
                if (*token != '(') { // это должна быть глобальная переменная
                    prog = tp; // возврат в начало объявления
                    decl_global();
                }
                else if (*token == '(') { // это должна быть функция
                    func_table[func_index].loc = prog;
                    func_table[func_index].ret_type = datatype;
                    strcpy(func_table[func_index].func_name, temp);
                    func_index++;
                    while (*prog != ')')
                        prog++;
                    prog++;
                    //сейчас prog указывает на открывающуюся
                    //фигурную скобку функции
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
// Интерпритация одного оператора или блока. Когда interp_block() возвращает
// управление после первого вызова, в main() встретилась последняя закрывающаяся
// фигурная скобка или оператор return.
void interp_block(void)
{
    int value;
    char block = 0;

    do {
        token_type = get_token();

        // При интерпритации одного операторавозврат
        //  после первой точки с запятой.

        // определение типа лексемы
        if (token_type == lcIDENTIFIER) {
            // Это не зарегестрированное слово,
            // обрабатывается выражение.
            putback(); // возврат лексемыво входной поток
            //  для дальнейшей обработки функцией eval_exp()
            eval_exp(&value); // обработка выражения
            if (*token != ';')
                sntx_err(SEMI_EXPECTED);
        }
        else if (token_type == lcBLOCK) {
            // если это граничитель блока
            if (*token == '{') // блок
                block = 1; // интерпритация блока, а не оператора
            else
                return; // это }, возврат
        }
        else // зарезервированное слово
            switch (tok) {
            case lcCHAR:
			case lcSTRUCT:
            case lcINT: // объявление локальной переменной
                putback();
                decl_local();
                break;
            case lcRETURN: // возврат из вызова функции
                func_ret();
                return;
            case lcIF: // обработка оператора if
                exec_if();
                break;
            case lcELSE: // обработка оператора else
                find_eob(); // поиск конца блока else
                // и продолжение выполнения
                break;
            case lcWHILE: // обработка цикла while
                exec_while();
                break;
            case lcDO: // обработка цикла do-while
                exec_do();
                break;
            case lcFOR: // обработка цикла for
                exec_for();
                break;
            case lcEND:
                exit(0);
            }
    } while (tok != lcFINISHED && block);
}
//---------------------------------------------------------------------------
// Возврат адреса точки входа данной функции. Возврат NULL, если не надена.
char* find_func(const char* name)
{
    register int i;

    for (i = 0; i < func_index; i++)
        if (!strcmp(name, func_table[i].func_name))
            return func_table[i].loc;

    return NULL;
}
//---------------------------------------------------------------------------
// Объявление глобальной переменной.
void decl_global(void)
{
	FCT_IDENT();
    int vartype;

    get_token(); // определение типа

    vartype = tok; // запоминание типа переменной

    do { // обработка списка
        global_vars[gvar_index].v_type = vartype;
        global_vars[gvar_index].value = 0; // инициализация нулем
        get_token(); // определение имени
        strcpy(global_vars[gvar_index].var_name, token);
        get_token();
        gvar_index++;
    } while (*token == ',');
    if (*token != ';')
        sntx_err(SEMI_EXPECTED);
	END_FCT();
}
//---------------------------------------------------------------------------
// Объявление локальной переменной.
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
		get_token(); // определение типа

	    i.v_type = tok;
	    i.value = 0; // инициализация нулем
	
	    do { // обработка списка
	        get_token(); // определение типа пременной
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
// Вызов функции.
void call(void)
{
	FCT_IDENT();
    char *loc, *temp;
    int lvartemp;

    loc = find_func(token); // найти точку входа функции
    if (loc == NULL)
        sntx_err(FUNC_UNDEF); // функция не определена
    else {
        lvartemp = lvartos; // запоминание индекса стека
        //  локальных переменных
        get_args(); // получение аргумента функции
        temp = prog; // запоминание адреса возврата
        func_push(lvartemp); // запоминание индекса стека
        //  локальных переменных
        prog = loc; // переустановка prog в начало функции
        get_params(); // загрузка параметров функции
        // значениями аргументов
        interp_block(); // интерпретация функции
        prog = temp; // восстановление prog
        lvartos = func_pop(); // восстановление стека
        // локальных переменных
    }
	END_FCT();
}
//---------------------------------------------------------------------------
// Заталкивание аргументов функций в стек локальных переменных.
void get_args(void)
{
    int value, count, temp[NUM_PARAMS];
    struct var_type i;

    count = 0;
    get_token();
    if (*token != '(')
        sntx_err(PAREN_EXPECTED);

    // обработка списка значений
    do {
        eval_exp(&value);
        temp[count] = value; // временное запоминание
        get_token();
        count++;
    } while (*token == ',');
    count--;
    // затолкнуть в local_var_stack в обратном порядке
    for (; count >= 0; count--) {
        i.value = temp[count];
        i.v_type = lcARG;
        local_push(i);
    }
}
//---------------------------------------------------------------------------
// Получение параметров функции.
void get_params(void)
{
    struct var_type* p;
    int i;

    i = lvartos - 1;
    do { // обработка списка параметров
        get_token();
        p = &local_var_stack[i];
        if (*token != ')') {
            if (tok != lcINT && tok != lcCHAR)
                sntx_err(TYPE_EXPECTED);

            p->v_type = token_type;
            get_token();

            // связывание имени пераметров с аргументом,
            //  уже находящимся в стеке локальных переменных
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
// Возврат из функции.
void func_ret(void)
{
    int value;

    value = 0;
    // получение возвращаемого значения, если оно есть
    eval_exp(&value);

    ret_value = value;
}
//---------------------------------------------------------------------------
// Затолкнуть локальную переменную.
void local_push(struct var_type i)
{
    if (lvartos > NUM_LOCAL_VARS)
        sntx_err(TOO_MANY_LVARS);

    local_var_stack[lvartos] = i;
    lvartos++;
}
//---------------------------------------------------------------------------
// Выталкивание индекса в стеке локальных переменных.
int func_pop(void)
{
    functos--;
    if (functos < 0)
        sntx_err(RET_NOCALL);
    return call_stack[functos];
}
//---------------------------------------------------------------------------
// Запись индекса в стек локальных переменных.
void func_push(int i)
{
    if (functos > NUM_FUNC)
        sntx_err(NEST_FUNC);
    call_stack[functos] = i;
    functos++;
}
//---------------------------------------------------------------------------
// Присваивание переменной значения.
void assign_var(char* var_name, int value)
{
    register int i;

    //проверка наличия локальной переменной
    for (i = lvartos - 1; i >= call_stack[functos - 1]; i--) {
        if (!strcmp(local_var_stack[i].var_name, var_name)) {
            local_var_stack[i].value = value;
            return;
        }
    }
    if (i < call_stack[functos - 1])
        // если переменная нелокальная,
        // ищем ее в таблице глобальных переменных
        for (i = 0; i < NUM_GLOBAL_VARS; i++)
            if (!strcmp(global_vars[i].var_name, var_name)) {
                global_vars[i].value = value;
                return;
            }
    sntx_err(NOT_VAR); //переменная не найдена
}
//---------------------------------------------------------------------------
// Получение значения переменной.
int find_var(char* s)
{
    register int i;

    // проверка наличия переменной
    for (i = lvartos - 1; i >= call_stack[functos - 1]; i--)
        if (!strcmp(local_var_stack[i].var_name, token))
            return local_var_stack[i].value;

    // в противном случае проверим,
    // может быть это глобальная переменная
    for (i = 0; i < NUM_GLOBAL_VARS; i++)
        if (!strcmp(global_vars[i].var_name, s))
            return global_vars[i].value;

    sntx_err(NOT_VAR); // переменная не найдена
    return -1;
}
//---------------------------------------------------------------------------
// Если индентификатор является переменной, то возвращается 1, иначе 0.
int is_var(char* s)
{
    register int i;

    // это локальная переменная ?
    for (i = lvartos - 1; i >= call_stack[functos - 1]; i--)
        if (!strcmp(local_var_stack[i].var_name, token))
            return 1;

    // если нет - поиск среди глобальных переменных
    for (i = 0; i < NUM_GLOBAL_VARS; i++)
        if (!strcmp(global_vars[i].var_name, s))
            return 1;

    return 0;
}
//---------------------------------------------------------------------------
// Выполнение оператора if.
void exec_if(void)
{
    int cond;

    eval_exp(&cond); // вычисление if-выражения

    if (cond) { // истина - интерпретация if-предложения
        interp_block();
    }
    else { // в противном случае пропуск if-предложения
        // и выполнение else-предложения, если оно есть
        find_eob(); // поиск конца блока
        get_token();

        if (tok != lcELSE) {
            putback(); // восстановление лексемы,
            // если else-предложение отсутсвует
            return;
        }
        interp_block();
    }
}
//---------------------------------------------------------------------------
// Выполнение цикла while.
void exec_while(void)
{
    int cond;
    char* temp;

    putback();
    temp = prog; // запоминание адреса начала цикла while
    get_token();
    eval_exp(&cond); // вычисление управляющего выражения
    if (cond)
        interp_block(); // если оно истинно, то выполнить
    // интерпритацию
    else { // в противном случае цикл пропускается
        find_eob();
        return;
    }
    prog = temp; // возврат к началу цикла
}
//---------------------------------------------------------------------------
// Выполнение цикла do.
void exec_do(void)
{
    int cond;
    char* temp;

    putback();
    temp = prog; // запоминание адреса начала цикла

    get_token(); // найти начало цикла
    interp_block(); // интерпритация цикла
    get_token();
    if (tok != lcWHILE)
        sntx_err(WHILE_EXPECTED);
    eval_exp(&cond); // проверка условия цикла
    if (cond)
        prog = temp; // если условие истинно,
    //то цикл выполняется, в противном случае происходит
    //выход из цикла
}
//---------------------------------------------------------------------------
// Поиск конца блока.
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
// Выполнение цикла for.
void exec_for(void)
{
    int cond;
    char *temp, *temp2;
    int brace;

    get_token();
    eval_exp(&cond); // инициализирующее выражение
    if (*token != ';')
        sntx_err(SEMI_EXPECTED);
    prog++; // пропуск ; */
    temp = prog;
    for (;;) {
        eval_exp(&cond); // проверка условия
        if (*token != ';')
            sntx_err(SEMI_EXPECTED);
        prog++; // пропуск ;
        temp2 = prog;

        // поиск начала тела цикла
        brace = 1;
        while (brace) {
            get_token();
            if (*token == '(')
                brace++;
            if (*token == ')')
                brace--;
        }

        if (cond)
            interp_block(); // если условие выполнено,
        // то выполнить интерпритацию
        else { // в противном случае обойти цикл
            find_eob();
            return;
        }
        prog = temp2;
        eval_exp(&cond); // вполнение инкремента
        prog = temp; // возврат в начало цикла
    }
}
