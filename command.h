
#ifndef command_h
#define command_h
#define MAXFILENAME 1024

// Command Data Structure
struct SimpleCommand {
	// Available space for arguments currently preallocated
	int _numberOfAvailableArguments;

	// Number of arguments
	int _numberOfArguments;
	char ** _arguments;

	SimpleCommand();
	void insertArgument( char * argument );
	void insertWildcard(char * arg);
	bool isWildcard(char *arg);
	void expandWildCard(char * prefix, char * suffix);
	char * convertToWildcard(char * arg);
	void bubbleSort(char ** array, int nEntries); 
	// void expandEnviron(char * argument);
	// void expandTilde(char * argument);
	void substring(char* oldstr, char* substr, int start, int end);
	char * str_replace (const char *string, const char *substr, const char *replacement);
};

struct Command {
	int _numberOfAvailableSimpleCommands;
	int _numberOfSimpleCommands;
	SimpleCommand ** _simpleCommands;
	char * _outFile;
	char * _inputFile;
	char * _errFile;
	int _background;

	int _inCounter;
	int _outCounter;
	int _errCounter;
	int _outFlag;
	int _errFlag;

	void prompt();
	void print();
	void execute();
	void clear(); 
	void end();
	void setEnv(char * arg1, char * arg2);
	void unsetEnv(char * arg);
	
	Command();
	void insertSimpleCommand( SimpleCommand * simpleCommand );

	static Command _currentCommand;
	static SimpleCommand *_currentSimpleCommand;
};

#endif
