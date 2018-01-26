#include <stdio.h>
#include "globals.h"
#include "lexer.h"
#include "debug.h"

/* Defines */
#define INI_LOG "ini.log"

FILE *ini_file;
static char *ini_buffer;
static int ini_buffer_size;

int ini_open(char *file_name)
{
	FILE *log_file;
	int nread = 0;
#ifndef DEBUG_BUILD
	log_file = freopen(INI_LOG, "w", stdout);
#endif	
	if ((ini_file = fopen(file_name, "rb")) == NULL)
	{
		return FALSE;
	}
	fseek(ini_file, 0, SEEK_END);
	ini_buffer_size = ftell(ini_file);

	if (ini_buffer_size < 0)
	{
		return 0;
	}

	fseek(ini_file, 0, SEEK_SET);
	prog = ini_buffer = (char*)malloc(sizeof(char)*ini_buffer_size + 1);
	nread = fread(ini_buffer, 1, ini_buffer_size, ini_file);
	prog[nread] = 0;
	return TRUE;
}

void ini_close()
{
	if (ini_file != NULL)
	{
		fclose(ini_file);
	}
}
char *get_value(char *key, char type)
{
	char *value, *ini_ptr = ini_buffer;
	
	if (ini_file != NULL)
		do
		{
			if (get_token() == lcIDENTIFIER)
			{
				if (!strcmp(key, token)){
					get_token();
					if (*token == '='){
						get_token();
						if (token_type == type){
							value = strdup(token);
							printf("value of key [%s] is %s\n", key, value);
							return value;
						}						
						else
						{

						}
					}
					else{
						break;
					}
				}continue;
			}break;
		} while (token_type != lcFINISHED);
error:
	return NULL;

}