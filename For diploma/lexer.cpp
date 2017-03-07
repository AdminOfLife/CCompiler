//---------------------------------------------------------------------------
#include <ctype.h>
//---------------------------------------------------------------------------
#include "globals.h"
#include "lexer.h"
#include "parser.h"

//---------------------------------------------------------------------------
struct commands { // таблица зарезервированных слов
	char command[20];
	char ltok;
} table[] = {
	// В эту таблицу
	"if", lcIF, // команды должны быть введены на нижнем регистре.
	"else", lcELSE,
	"for", lcFOR,
	"do", lcDO,
	"while", lcWHILE,
	"char", lcCHAR,
	"int", lcINT,
	"return", lcRETURN,
	"switch", lcSWITCH,
	"end", lcEND,
	"", lcEND // конец таблицы
};
//---------------------------------------------------------------------------
// —читывание лексемы из входного потока.
int get_token(void)
{

	register char* temp;

	token_type = 0;
	tok = 0;

	temp = token;
	*temp = '\0';

	// пропуск пробелов, символов табул¤ции и пустой строки
	while (iswhite(*prog) && *prog)
		++prog;

	if (*prog == '\r') {
		++prog;
		++prog;
		// пропуск пробелов
		while (iswhite(*prog) && *prog)
			++prog;
	}

	if (*prog == '\0') { // конец файла
		*token = '\0';
		tok = lcFINISHED;
		return (token_type = lcDELIMITER);
	}

	if (strchr("{}", *prog)) { // ограничение блока
		*temp = *prog;
		temp++;
		*temp = '\0';
		prog++;
		return (token_type = lcBLOCK);
	}

	// поиск комментариев
	if (*prog == '/')
		if (*(prog + 1) == '*') { // это комментарий
			prog += 2;
			do { // найти конец комментари¤
				while (*prog != '*')
					prog++;
				prog++;
			} while (*prog != '/');
			prog++;
		}

		if (strchr("!<>=", *prog)) { // возможно, это
			// оператор сравнени¤
			switch (*prog) {
case '=':
	if (*(prog + 1) == '=') {
		prog++;
		prog++;
		*temp = EQ;
		temp++;
		*temp = EQ;
		temp++;
		*temp = '\0';
	}
	break;
case '!':
	if (*(prog + 1) == '=') {
		prog++;
		prog++;
		*temp = NE;
		temp++;
		*temp = NE;
		temp++;
		*temp = '\0';
	}
	break;
case '<':
	if (*(prog + 1) == '=') {
		prog++;
		prog++;
		*temp = LE;
		temp++;
		*temp = LE;
	}
	else {
		prog++;
		*temp = LT;
	}
	temp++;
	*temp = '\0';
	break;
case '>':
	if (*(prog + 1) == '=') {
		prog++;
		prog++;
		*temp = GE;
		temp++;
		*temp = GE;
	}
	else {
		prog++;
		*temp = GT;
	}
	temp++;
	*temp = '\0';
	break;
			}
			if (*token)
				return (token_type = lcDELIMITER);
		}

		if (strchr("+-*^/%=;(),'", *prog)) { // разделитель
			*temp = *prog;
			prog++; // продвижение на следующую позицию
			temp++;
			*temp = '\0';
			return (token_type = lcDELIMITER);
		}

		if (*prog == '"') { // строка в кавычках
			prog++;
			while (*prog != '"' && *prog != '\r')
				*temp++ = *prog++;
			if (*prog == '\r')
				sntx_err(SYNTAX);
			prog++;
			*temp = '\0';
			return (token_type = lcSTRING);
		}

		if (isdigit(*prog)) { // число
			while (!isdelim(*prog))
				*temp++ = *prog++;
			*temp = '\0';
			return (token_type = lcNUMBER);
		}

		if (isalpha(*prog)) { // переменна¤ или оператор
			while (!isdelim(*prog))
				*temp++ = *prog++;
			token_type = lcTEMP;
		}

		*temp = '\0';

		//эта строка ¤вл¤етс¤ оператором или переменной?
		if (token_type == lcTEMP) {
			tok = look_up(token); // преобразовать во внутренее представление
			if (tok)
				token_type = lcKEYWORD; // это зарезервированное слово
			else
				token_type = lcIDENTIFIER;
		}
		return token_type;
}
//---------------------------------------------------------------------------
// ¬озврат лексемы во входной поток.
void putback(void)
{
	char* t;

	t = token;
	for (; *t; t++)
		prog--;
}
//---------------------------------------------------------------------------
// ѕоиск внутреннего представлени¤ лексемы в таблице лексем.
int look_up(char* s)
{
	register int i;
	char* p;

	// преобразование в нижний регистр
	p = s;
	while (*p) {
		*p = tolower(*p);
		p++;
	}

	// есть ли лексемы в таблице?
	for (i = 0; *table[i].command; i++) {
		if (!strcmp(table[i].command, s))
			return table[i].ltok;
	}
	return 0; // незнакомый оператор
}