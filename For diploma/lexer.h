#ifndef LEXER_H
#define LEXER_H
//---------------------------------------------------------------------------
// сюда можно добавить дополнительные лексемы зарезервированных слов
enum tokens { 
	lcARG,
    lcCHAR,
    lcINT,
	lcSTRUCT,
	lcTYPEDEF,
    lcIF,
    lcELSE,
    lcFOR,
    lcDO,
    lcWHILE,
    lcSWITCH,
    lcRETURN,
    lcEOL,
    lcFINISHED,
    lcEND };
//---------------------------------------------------------------------------
enum tok_types { 
	lcDELIMITER,
    lcIDENTIFIER,
    lcNUMBER,
    lcKEYWORD,
    lcTEMP,
    lcSTRING,
    lcBLOCK 
	};
//---------------------------------------------------------------------------
// сюда можно добавить дополнительные двухсимвольные операторы, например, ->
enum double_ops { LT = 1,
    LE,
    GT,
    GE,
    EQ,
    NE 
	};
//---------------------------------------------------------------------------
struct define_tab
{
	char *name;
	char *value;
};

int get_token(void);
void putback(void);
int iswhite(char c);
int isdelim(char c);
int look_up(char* s);
struct define_tab *in_deftab(char *token);

#endif // LEXER_H
