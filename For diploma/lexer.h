#ifndef LEXER_H
#define LEXER_H
//---------------------------------------------------------------------------
// ���� ����� �������� �������������� ������� ����������������� ����
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
// ���� ����� �������� �������������� �������������� ���������, ��������, ->
enum double_ops { LT = 1,
    LE,
    GT,
    GE,
    EQ,
    NE 
	};
//---------------------------------------------------------------------------
int get_token(void);
void putback(void);
int iswhite(char c);
int isdelim(char c);
int look_up(char* s);

#endif // LEXER_H
