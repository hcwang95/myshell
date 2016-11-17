/*
Student name and No.: 	WANG Haicheng
Student  No.:			3035140108
Development platform:	Ubuntu 14.04
Last modified date:		Oct. 21
Compile: "make"
*/

// set up for some important flags or variables
#define BUFFERSIZE 1024
#define MAXARGSIZE 30
#define PIPE 124
#define BACKGROUND 38
#define WHITESPACE 32
#define EXIT 0
#define CONTINUEFOREGROUND 1
#define CONTINUEBACKGROUND 6
#define EXIT_TOO_MANY_ARGUMENTS 2
#define ISBACKGROUND 3
#define NOBACKGROUND 4
#define WRONGBACKGROUND 5
#define CONTINUEWITHOUTEXECUTE 7
#define GETSIGINT 8
#define INCOMLETEPIPE 9
#define GETSIGCHILD 10
#define TIMEXBACKGROUND 11
#define TIMEXSTANDALONE 12
#define TIMEXINCOMLETEPIPE 13

// set up for global variables
pid_t foreground_pid;
pid_t *pidList;
int pidList_size;
pid_t * return_background_pid_list;
int return_background_size;
unsigned long long start_time;
unsigned long long * start_time_pipe;

// define instruction container
typedef struct Instruction
{
	char* fileName;
	char** arg;
	int argSize;
} Instruction;

typedef struct InstructionList
{
	Instruction ** list;
	size_t instructionSize;
} InstructionList;