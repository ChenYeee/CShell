
/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <regex.h>
#include <assert.h>
#include <pwd.h>

#include "command.h"

extern char ** environ;
int maxEntries = 20;
int nEntries = 0;
char ** array = (char**) malloc(maxEntries*sizeof(char*));


SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertWildcard(char * arg)
{
	expandWildCard(NULL, arg);

	bubbleSort(array, nEntries);
	for (int i = 0; i < nEntries; i++) {
		 // printf("array[%d] = %s\n", i, array[i]);
		insertArgument(array[i]);
	}
}

bool
SimpleCommand::isWildcard(char *arg) 
{
	if(strchr(arg, '*') == NULL && strchr(arg, '?') == NULL){
		return false;
	}
	return true;
}

char *
SimpleCommand::convertToWildcard(char * arg)
{
	char * reg = (char *) malloc(strlen(arg) * 2 + 10);
	char * a = arg;
	char * r = reg;
	*r = '^'; r++;
	while (*a) {
		if(*a == '*') { *r = '.'; r++ ; *r = '*'; r++; }
		else if(*a == '?') { *r = '.'; r++;}
		else if(*a == '.') { *r = '\\'; r++; *r = '.'; r++;}
		else { *r = *a; r++; }
		a++;
	}
		*r = '$'; r++; *r = 0;
		return reg;
}

void
SimpleCommand::expandWildCard(char * prefix, char * suffix) 
{
   	// printf("enter expandWildCard, the prefix is %s\n", prefix);
	// printf("enter expandWildCard, the suffix is %s\n", suffix);

	if(suffix[0] == 0) {
		if(nEntries == maxEntries) {
		maxEntries *=2;
		// printf("realloc array\n");
		array = (char **) realloc(array, maxEntries*sizeof(char*));
		assert(array!=NULL);
		}
		array[nEntries] = (char *)calloc(1, MAXFILENAME);
				// printf("calloc array entry %d\n", nEntries);
		strcpy(array[nEntries], prefix);
		 // printf("2The inserted prefix is %s\n", array[nEntries]);
		nEntries++;
		return;
	}
  // printf("1\n");

	char * s = strchr(suffix, '/');
	char component[MAXFILENAME];
	if(s != NULL) {
		strncpy(component, suffix, s - suffix);
		suffix = s + 1;
	} 
	else {
		strcpy(component, suffix);
		suffix = suffix + strlen(suffix);
	}
  // printf("2\n");

	char newPrefix[MAXFILENAME];
	if(strchr(component, '*') == NULL && strchr(component, '?') == NULL) {
		if(prefix == NULL) {
    		sprintf(newPrefix,"%s", component);
    	} else {
    		sprintf(newPrefix,"%s/%s", prefix, component);
    	}
		expandWildCard(newPrefix, suffix);
		return;
	} 

	char * reg = convertToWildcard(component); 
	regex_t re;
	int ret  = regcomp(&re, reg,  REG_EXTENDED|REG_NOSUB);
	if(ret != 0) {
		perror("regcomp");
		exit(1);
	}
  // printf("3\n");

	const char * dir;
	if(prefix == NULL) {
		// printf("ever in enter .?\n");
		dir = ".";
	} else {
		dir = prefix;
	}

	DIR * d = opendir(dir);
		if (d == NULL) {
			return;
		}
  // printf("4\n");

		regmatch_t match;
		struct dirent * ent;

		while ((ent = readdir(d)) != NULL) {
			    // printf("in while loop, the file name is %s\n", ent->d_name);
			if (regexec(&re, ent->d_name, 1, &match, 0) == 0) {
				   // printf("the file name is %s\n", ent->d_name);
				if (ent->d_name[0] == '.') {
    				if (component[0] == '.') {
    					 // printf("Mathc hidden file\n");
    					if(prefix == NULL) {
    						sprintf(newPrefix,"%s", ent->d_name);
    					} else {
    						sprintf(newPrefix,"%s/%s", prefix, ent->d_name);
    					}
    					expandWildCard(newPrefix, suffix); 					
					}
  				}
				else {
					if(prefix == NULL) {
    					sprintf(newPrefix,"%s", ent->d_name);
    				} else {
    					sprintf(newPrefix,"%s/%s", prefix, ent->d_name);
    				}
    				expandWildCard(newPrefix, suffix); 
				}
			}

		}

		free(reg);
		regfree(&re);
		closedir(d);
	
		// printf("array's length is %d\n", nEntries);
		// for (int i = 0; i < nEntries; i++) {
			 // printf("before sort: array[%d] = %s\n", i, array[i]);
		// }
}

void
SimpleCommand::bubbleSort(char ** arr, int n) 
{
 	for(int i = 0; i < n - 1; i++) {
 		for(int j = 0; j < n - i - 1; j++) {
 			  // printf("	in bubbleSort: i = %d, j = %d\n", i, j );
 			if(strcmp(arr[j], arr[j + 1]) > 0) {
 				int length = strlen(arr[j]);
 				char *temp = (char *)calloc(1, length+10);
 				strcpy(temp, arr[j]);
 			// strcpy(tmp, arr[j]);
 			strcpy(arr[j], arr[j + 1]);
 			strcpy(arr[j + 1], temp);
 			free(temp);
 			}
 		}
 	}
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;

		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}

	if(strchr(argument, '$') != NULL 
		&& strchr(argument, '{') != NULL && strchr(argument, '}') != NULL) {
		// expandEnviron(argument);
		const char * buffer = "\\$\\{[^}][^}]*\\}";
		const int n_matches = 10;
		const char * p = argument;

		// buffer = strdup("\\$\\{[^}][^}]*\\}");
		regex_t re;

		int ret  = regcomp(&re, buffer, REG_EXTENDED);
		if(ret != 0) {
			perror("regcomp: Bad regular expression");
			exit(1);
		}

		regmatch_t match[n_matches];
		while(1) {
				// printf("ia2\n");

			int nomatch = regexec(&re, p, n_matches, match, 0);
			if(nomatch) {
				break;
			}
			 for(int i = 0; i < n_matches; i++) {
				int start;
				int end;
				if(match[i].rm_so == -1) {
					break;
				}
				start = match[i].rm_so + (p - argument);
				end = match[i].rm_eo + (p - argument) - 1;

				char * matchstr = (char *) calloc(1, end - start + 2);
				substring(argument, matchstr, start, end);
				char * substr = (char *) calloc(1, end - start - 1);
				substring(matchstr, substr, 2, end - start - 1);
				char * replacement;
		 		replacement = strdup(getenv(substr));
		 		if(replacement == NULL) {
					perror("getenv");
					exit(1);
				}
				argument = strdup(str_replace(argument, matchstr, replacement));
			
				free(matchstr);
				free(substr);
				free(replacement);
			 }
			p += match[0].rm_eo;
		}
		regfree(&re);
	}

	if(argument[0] == '~') {
		// expandTilde(argument);
		char * ptr = strchr(argument, '/');
		if(strlen(argument) == 1 || argument[1] == '/') {
			const char * substr = strdup("~");
			char * replacement;
		 	replacement = strdup(strcat(getenv("HOME"),"/"));
		 	if(replacement == NULL) {
				perror("getenv");
				exit(1);
			}
			argument = strdup(str_replace(argument, substr, replacement));
		} else if (ptr != NULL){
			char * substr = (char * ) calloc(1, ptr - argument + 1);
			char * user = (char * ) calloc(1, ptr - argument);
			substring(argument, substr, 0, ptr - argument - 1);
			substring(substr, user, 1, strlen(substr) - 1);

			struct passwd * pw = getpwnam(user);
		 	if(pw == NULL) {
				perror("getownam");
				exit(1);
			}

			char * replacement;
		 	replacement = strdup(pw->pw_dir);
			argument = strdup(str_replace(argument, substr, replacement));

			free(substr);
			free(user);

		} else {
			char * substr = strdup(argument);
			char * user = (char * ) calloc(1, strlen(argument));
			substring(substr, user, 1, strlen(substr) - 1);

			struct passwd * pw = getpwnam(user);
		 	if(pw == NULL) {
				perror("getownam");
				exit(1);
			}

			char * replacement;
		 	replacement = strdup(pw->pw_dir);
			argument = strdup(str_replace(argument, substr, replacement));

			free(user);
		}

	}

	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

void
SimpleCommand::substring(char * oldstr, char * substr, int start, int end)
{	
	int c = 0;
	int l = end - start + 1;
 
    while (c < l) {
      substr[c] = oldstr[start + c];
      c++; 
   }
}

/*This function is cited from 
http://coding.debuntu.org/c-implementing-str_replace-replace-all-occurrences-substring
and is editted */
char *
SimpleCommand::str_replace(const char *string, const char *substr, const char *replacement)
{
	char *tok = NULL;
	char *newstr = NULL;
  	char *oldstr = NULL;
  	newstr = strdup(string);
  	while ((tok = strstr(newstr, substr))){
    	oldstr = newstr;
    	newstr = (char *) malloc(strlen(oldstr) - strlen(substr) + strlen(replacement) + 1 );
    	/*failed to alloc mem, free old string and return NULL */
    	if (newstr == NULL){
      		free(oldstr);
      		perror("malloc");
      		exit(1);
    	}
    	memcpy(newstr, oldstr, tok - oldstr);
    	memcpy(newstr + (tok - oldstr), replacement, strlen(replacement));
    	memcpy(newstr + (tok - oldstr) + strlen(replacement), tok + strlen(substr), 
    		strlen(oldstr) - strlen(substr) - (tok - oldstr));
    	memset(newstr + strlen(oldstr) - strlen(substr) + strlen(replacement) , 0, 1);
    	free(oldstr);
  	}
  	// printf("newstr = %s\n", newstr);
  	return newstr;
}


Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;

	_inCounter = 0;
	_outCounter = 0;
	_errCounter = 0;
	_outFlag = 0;
	_errFlag = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command::clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}



	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	// if (nEntries > 0) {
	// 	free( array);
	// }
	maxEntries = 20;
	nEntries = 0;

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;

	_inCounter = 0;
	_outCounter = 0;
	_errCounter = 0;
	_outFlag = 0;
	_errFlag = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
		printf( "\n" );
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}


void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

	// Print contents of Command data structure
	// print();

	// check for ambiguous redirect
	if(_inCounter > 1) {
		printf("Ambiguous input redirect.\n");
	}
	else if(_outCounter > 1) {
		printf("Ambiguous output redirect.\n");
	}
	else if(_errCounter > 1) {
		printf("Ambiguous error redirect.\n");
	}
	else {
		// Save default input, output, and error 
		int defaultin = dup(0);
		int defaultout = dup(1);
		int defaulterr = dup(2);
		// set initial input
		int fdin;
		if(_inputFile != 0) {
			fdin = open(_inputFile, O_RDONLY);
		}
		else {
			fdin = dup(defaultin);
		}

		int pid;
		int fdout;
		int fderr;

		// if error redirect exists
		if(_errFile != 0) {
			if(_errFlag == 0) {
				fderr = open(_errFile, O_WRONLY|O_CREAT|O_TRUNC, 0600);
			}
			else { 
				fderr = open(_errFile, O_WRONLY|O_CREAT|O_APPEND, 0600);
			}
		}
		else { 
			fderr = dup(defaulterr);
		}

		for(int i = 0; i < _numberOfSimpleCommands; i++) {
			// printf("number of simple commands %d\n", _numberOfSimpleCommands);
			 // printf("Now executing %s, i = %d\n", _simpleCommands[i]->_arguments[0], i);

			dup2(fdin, 0);
			close (fdin);

			if(i == _numberOfSimpleCommands - 1) {
				// if output redirect exists
				if(_outFile != 0) {
					if(_outFlag == 0) {
						fdout = open(_outFile, O_WRONLY|O_CREAT|O_TRUNC, 0600);
					}
					else { 
						fdout = open(_outFile, O_WRONLY|O_CREAT|O_APPEND, 0600);
					}
				}
				else { 
					fdout = dup(defaultout);
				}
			}
			else {
				int fdpipe[2];
				if ( pipe(fdpipe) == -1) {
					perror("pipe");
					exit( 2 );
				}
				fdout = fdpipe[1];
				fdin = fdpipe[0];
			}

			dup2(fdout, 1);
			close(fdout);
			dup2(fderr, 2);
			close(fderr);

			if (!strcmp(_simpleCommands[i]->_arguments[0], "cd")) {
				if(_simpleCommands[i]->_numberOfArguments == 1) {
					const char * homePath = strdup(getenv("HOME"));
					if (chdir(homePath) == -1) {
						perror("chdir");
					}
				} else {
					const char * path = (strdup(_simpleCommands[i]->_arguments[1]));
					if (chdir(path) == -1) {
						perror("chdir");
					}
				}
				break;
			}

			if (!strcmp(_simpleCommands[i]->_arguments[0], "setenv")) {	
				setEnv(_simpleCommands[i]->_arguments[1], _simpleCommands[i]->_arguments[2]);
				break;
			}

			if (!strcmp(_simpleCommands[i]->_arguments[0], "unsetenv")) {	
				unsetEnv(_simpleCommands[i]->_arguments[1]);
				break;
			}

			if (!strcmp(_simpleCommands[i]-> _arguments[0], "exit")) {
				end();
			}
			
			pid = fork();
			if ( pid == -1 ) {
				perror("fork\n");
				exit( 2 );
			}

			if (pid == 0) {
				close (defaultin);
				close (defaultout);
				close (defaulterr);

				if (!strcmp(_simpleCommands[i]->_arguments[0], "printenv")) {
    				char ** p = environ;
    				while (* p != NULL) {
    					printf("%s\n", * p);
    					p++;
    				}
    				fflush(stdout);
    				_exit(1);
				}

				execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
				perror("execvp");
				_exit(1);
			}
		 	// printf("Finish %s\n", _simpleCommands[i]->_arguments[0]);
		}

		dup2(defaultin, 0);
		dup2(defaultout, 1);
		dup2(defaulterr, 2);
		close(defaultin);
		close(defaultout);
		close(defaulterr);

		if(_background == 0) {
			waitpid(pid, 0, 0);
		}
	}
	// Clear to prepare for next command
	clear();

	// Print new prompt
	prompt();
}

// Shell implementation

void
Command::setEnv(char * arg1, char * arg2)
{
	char * str1;
	char * str2;
	str1 = strdup(arg1);
	str2 = strdup(arg2);
	// dprintf(1, "The string is %s\n", string);
	int ret = setenv(str1, str2, 1);

	free(str1);
	free(str2);

	if(ret != 0) {
		perror("setenv\n");
		exit(2);
	}
}

void
Command::unsetEnv(char * arg)
{
	int pos = 0;
	while(environ[pos] != NULL) {
		//printf("1st while in unsetEnv: %d\n", pos);
		if(strstr(environ[pos], arg) != NULL) {
			break;
		}
		pos++;
	}

	int i = pos;
	if(environ[i + 1] == NULL) {
		delete[] environ[i];
		environ[i] = NULL;
	}
	else {
		while(environ[i + 1] != NULL) {
		//	printf("2nd while in unsetEnv: %d\n", i+1);
			environ[i] = environ[i + 1];
			i++;
		}
		delete[] environ[i + 1];
		environ[i + 1] = NULL;
	}
}

void
Command::prompt()
{
	// printf("myshell>");
 // 	fflush(stdout);
	if(isatty(0)) {
 		printf("myshell>");
 		 fflush(stdout);

 	}
}

void
Command::end()
{
	free(array);
	_exit(2);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

extern "C" void disp(int sig) {
	printf("\n");
}

void
killzombie(int sig) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main()
{	
	signal(SIGINT, disp);
	Command::_currentCommand.prompt();

	struct sigaction signalAction;
	signalAction.sa_handler = killzombie;
	sigemptyset(&signalAction.sa_mask);
	signalAction.sa_flags = SA_RESTART;
	int error = sigaction(SIGCHLD, &signalAction, NULL );
	
	if(error) {
		perror("sigaction");
		exit(-1);
	}
	
	yyparse();
}

