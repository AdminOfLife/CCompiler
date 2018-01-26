//---------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
//---------------------------------------------------------------------------
#include "globals.h"
#include "lexer.h"
#include "parser.h"

//---------------------------------------------------------------------------
struct commands { // ������� ����������������� ����
	char command[20];
	char ltok;
} table[] = {
	// � ��� �������
	"if", lcIF, // ������� ������ ���� ������� �� ������ ��������.
	"else", lcELSE,
	"for", lcFOR,
	"do", lcDO,
	"while", lcWHILE,
	"char", lcCHAR,
	"int", lcINT,
	"return", lcRETURN,
	"switch", lcSWITCH,
	"struct", lcSTRUCT,
	/*typedef", lcTYPEDEF,*/
	"end", lcEND,
	"", lcEND // ����� �������
};

#define DESC(type) {#type, type}
struct token_type_desc
{
	char *desc;
	tok_types type;
}desc[] = {
	DESC(lcDELIMITER),
	DESC(lcIDENTIFIER),
	DESC(lcNUMBER),
	DESC(lcKEYWORD),
	DESC(lcTEMP),
	DESC(lcSTRING),
	DESC(lcBLOCK)
};

struct define_tab def_tab[255];
//---------------------------------------------------------------------------
char header[256];
int define_tab_indx;
//---------------------------------------------------------------------------
char *get_type_desc(int token_type)
{
	for (int i = 0; i < sizeof(desc) / sizeof(token_type); i++)
	{
		if (desc[i].type == token_type)
		{
			return desc[i].desc;
		}
	}
	return NULL;
}
struct define_tab *in_deftab(char *token)
{
	register int i;
	char* p;

	// �������������� � ������ �������
	p = token;
	while (*p) {
		*p = tolower(*p);
		p++;
	}

	// ���� �� ������� � �������?
	for (i = 0; *def_tab[i].name; i++) {
		if (!strcmp(def_tab[i].name, token))
			return &(def_tab[i]);
	}
	return 0; // ���������� ��������
}
//---------------------------------------------------------------------------
// ���������� ������� �� �������� ������.
int get_token(void)
{
	
	register char* temp;

	token_type = 0;
	tok = 0;

	temp = token;
	*temp = '\0';
	if (*prog == '\0') { // ����� �����
		*token = '\0';
		tok = lcFINISHED;
		DEBUG_TOKEN();
		return (token_type = lcDELIMITER);
	}
	// ������� ��������, �������� �������� � ������ ������
	while (iswhite(*prog) && *prog)
		++prog;

	if (*prog == '\r') {
		++prog;
		++prog;
		// ������� ��������
		while (iswhite(*prog) && *prog)
			++prog;
	}



	if (strchr("{}", *prog)) { // ����������� �����
		*temp = *prog;
		temp++;
		*temp = '\0';
		prog++;
		DEBUG_TOKEN();
		return (token_type = lcBLOCK);
	}

	// ����� ������������
	if (*prog == '/')
		if (*(prog + 1) == '*') { // ��� �����������
			prog += 2;
			do { // ����� ����� ����������
				while (*prog != '*')
					prog++;
				prog++;
			} while (*prog != '/');
			prog++;
		}
	if (*prog == '#')
	{
		*temp++ = *prog++;
		*temp = 0;
		DEBUG_TOKEN();
		return (token_type = lcDELIMITER);
	}

	if (strchr("!<>=", *prog)) { // ��������, ���
		// �������� ��������
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
		{
			DEBUG_TOKEN();
			return (token_type = lcDELIMITER);
		}			
			
	}

	if (strchr("+-*^/%=;(),'", *prog)) { // �����������
		*temp = *prog;
		prog++; // ����������� �� ��������� �������
		temp++;
		*temp = '\0';
		DEBUG_TOKEN();
		return (token_type = lcDELIMITER);
	}

	if (*prog == '"') { // ������ � ��������
		prog++;
		while (*prog != '"' && *prog != '\r')
			*temp++ = *prog++;
		if (*prog == '\r')
			sntx_err(SYNTAX);
		prog++;
		*temp = '\0';
		DEBUG_TOKEN();
		return (token_type = lcSTRING);
	}

	if (isdigit(*prog)) { // �����
		while (!isdelim(*prog))
			*temp++ = *prog++;
		*temp = '\0';
		DEBUG_TOKEN();
	return (token_type = lcNUMBER);
	}

	if (isalpha(*prog)) { // ��������� ��� ��������
		while (!isdelim(*prog))
			*temp++ = *prog++;
		token_type = lcTEMP;
	}

	*temp = '\0';

	//��� ������ ������ ���������� ��� ����������?
	if (token_type == lcTEMP) {
		tok = look_up(token); // ������������� �� ��������� �������������
		if (tok)
			token_type = lcKEYWORD; // ��� ����������������� �����
		else
			token_type = lcIDENTIFIER;
	}
	DEBUG_TOKEN();
	return token_type;
}

//---------------------------------------------------------------------------
// ������� ������� �� ������� �����.
void putback(void)
{
	char* t;

	t = token;
	for (; *t; t++)
		prog--;
}
//---------------------------------------------------------------------------
// ����� ����������� ������������ ������� � ������� ������.
int look_up(char* s)
{
	register int i;
	char* p;

	// �������������� � ������ �������
	p = s;
	while (*p) {
		*p = tolower(*p);
		p++;
	}

	// ���� �� ������� � �������?
	for (i = 0; *table[i].command; i++) {
		if (!strcmp(table[i].command, s))
			return table[i].ltok;
	}
	return 0; // ���������� ��������
}
