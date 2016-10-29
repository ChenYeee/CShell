
/*
 *
 * CS-252
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "y.tab.h"

static  void yyunput (int c, char *buf_ptr  );
static void substr(char * oldstr, char * substr, int start, int end);

void myunputc(int c) {
    unput(c);
}

void substring(char * oldstr, char * substr, int start, int end)
{	
	int c = 0;
	int l = end - start + 1;
 
    while (c < l) {
      substr[c] = oldstr[start + c];
      c++; 
   }
}

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t] 	{
			/* Discard spaces and tabs */
     	}	

"|"		{
			return PIPE;
		}

"<"		{
			return LESS;
		}

">>&"	{
			return GREATGREATAMPERSAND;
		}

">&"	{
			return GREATAMPERSAND;
		}

">>"	{
			return GREATGREAT;
		}

">" 	{
			return GREAT;
		}

"&"		{
			return AMPERSAND;
		}
"exit"  {
			return EXIT;
		}

\"[^\n\"]*\"	{
					yylval.string_val = strdup(yytext);
					char *sub = (char * )calloc(1, strlen(yylval.string_val) - 2);
					substring(yylval.string_val, sub, 1, strlen(yylval.string_val) - 2);
					strcpy(yylval.string_val, sub);
					free(sub);

					return WORD;
				}

[^ \t\n<>\&\|]*(\\.[^ \t\n<>\&\|]*)* {
	/* Assume that file names have only alpha chars */
	yylval.string_val = strdup(yytext);

	char * newstr = (char * )calloc(1, strlen(yylval.string_val));
	char * r = newstr;
	char * p = yylval.string_val;

	while(*p) {
		if(*p == '\\' && *(p + 1) != '\0') {*r = *(p + 1); r++; p++;}
		else {*r = *p; r++; }
		p++;
	}

	strcpy(yylval.string_val, newstr);
	free(newstr);
	return WORD;
}
