
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [ | cmd [args]*]* 
 * 	[ [> filename] [< filename] [>> filename] [>& filename] [>>& filename] ]*
 * 	[&]
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT GREATGREAT GREATAMPERSAND GREATGREATAMPERSAND LESS
		NEWLINE AMPERSAND PIPE EXIT

%union	{
		char   *string_val;
	}

%{
//#define yylex yylex
#include <stdio.h>
#include "command.h"
void yyerror(const char * s);
int yylex();

%}

%%

goal:	
	command_list
	;

command_list: 
	command_line
	| command_list command_line
	;

command_line: 
	pipe_list iomodifier_list background_opt NEWLINE {
		// printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE {
		Command::_currentCommand.execute();
	}
	| EXIT {
		Command::_currentCommand.end();	
	}
	| error NEWLINE {
		yyerrok;
	}
    ;

pipe_list:	
	pipe_list PIPE command_and_args 
	| command_and_args 
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand);
	}
	;

arg_list:
	arg_list argument 
	| /* can be empty */ 
	;

argument:
	WORD {
//           printf("   Yacc: insert argument \"%s\"\n", $1);
         if (!Command::_currentSimpleCommand->isWildcard( $1 )) {
         	Command::_currentSimpleCommand->insertArgument( $1 );
         } else {
         	Command::_currentSimpleCommand->insertWildcard( $1 );
         }
          	
//	       	
	}
	;

command_word:
	WORD {
//                 printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifier_list:
	iomodifier_list iomodifier_opt
	|/* can be empty */
	;

iomodifier_opt:
	GREATGREATAMPERSAND WORD {
		// printf("   Yacc: insert appending stderr output \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._errCounter++;
		Command::_currentCommand._errFlag = 1;
	}
	| GREATGREAT WORD {
		// printf("   Yacc: insert appending output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._outCounter++;
		Command::_currentCommand._outFlag = 1;
	}
	| GREATAMPERSAND WORD {
		// printf("   Yacc: insert stderr output \"%s\"\n", $2);
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._errCounter++;
	}
	| GREAT WORD {
		// printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._outCounter++;

	}
	| LESS WORD {
		// printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
		Command::_currentCommand._inCounter++;
	}
	;

background_opt:
	AMPERSAND {
		// printf("   Yacc: insert background \n");
		Command::_currentCommand._background = 1;
	}
	| /* can be empty */
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
