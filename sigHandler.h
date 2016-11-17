/*
Student name and No.: 	WANG Haicheng
Student  No.:			3035140108
Development platform:	Ubuntu 14.04
Last modified date:		Oct. 21
Compile: "make"
*/

/* 
	helper function to get process command line
	get command name from /proc/{PID}/stat
*/
char* get_process_name_by_pid(const int pid)
{
    char* name = (char*)calloc(1024,sizeof(char));
    char * token;
    if(name){
        sprintf(name, "/proc/%d/stat",pid);
        FILE* f = fopen(name,"r");
        if(f){
            size_t size;
            char deliminator = 32;
            
            size = fread(name, sizeof(char), 1024, f);
           	token = strtok(name, &deliminator);
			token = strtok(NULL, &deliminator);
			token[strlen(token) - 1] = 0;
			token++;
            fclose(f);
        }
    }
    return token;
}

/*
	helper function to get the index in an array given a element
*/
int get_index( const int * pidList, int size, int value)
{
    int index = 0;

    while ( index < size && pidList[index] != value ) ++index;

    return ( index == size ? -1 : index );
}

/*
	helper function to base on special integer get from siginfo to generate valid time
*/
char* prepare_time(int time)
{
	char* expr = (char*)calloc(100,sizeof(char));
	float real_time = (double)time / 100 ;
	sprintf(expr,"%.2f s", real_time);
	return expr;
}

/* 
	helper function to check if one element is in the array or not
*/
bool in(pid_t pid, pid_t* list, int size){
	int i;
    for (i=0; i < size; i++) {
        if (list[i] == pid)
            return true;
    }
    return false;
}

pid_t* addIn(pid_t* pidList, int size, pid_t element)
{
	
	return pidList;
}
/*
 	this is the main program SIGINT handler
 	will provide the required performance as the oracle program shows
 */
void sigint_handler (int signum) 
{
	fprintf(stdout, "\n");
	fprintf(stdout, "## myshell $ ");
}

/*
	this is the SIGCHILD handler dealing with mainly with background processes
	when a non-pipe foreground process is running (also deal with background running)
	return required message and appropriately handle zombie processes
*/
void sigchild_background_handler(int signum, siginfo_t *sig, void *context) 
{	
	// here we judge if the SIGCHILD is from current foreground child then we do not print
	if (sig->si_pid == foreground_pid)
	{
	}
	else
	{
		size_t status;
		char* name = get_process_name_by_pid((sig->si_pid));
		printf("[%d] %s Done\n", (int)(sig->si_pid), name);
		waitpid ((int)(sig->si_pid), &status, 0);
	}
}


void sigchild_foreground_running_background_handler(int signum, siginfo_t *sig, void *context) 
{	
	// here we judge if the SIGCHILD is from current foreground child then we do not print
	if (sig->si_pid == foreground_pid)
	{
	}
	else
	{
		size_t status;
		char* name = get_process_name_by_pid((sig->si_pid));
		// register return background process in order to collect them after handler returns
		if (return_background_size == 0)
		{
			return_background_pid_list = (pid_t *)malloc(sizeof(pid_t)*10);
		}
		else if (return_background_size > 0 && return_background_size % 10 == 0)
		{
			return_background_pid_list = (pid_t *)realloc(return_background_pid_list,sizeof(pid_t)*(return_background_size/10 + 1));
		}
		
		return_background_pid_list[return_background_size] = sig->si_pid;
		return_background_size++;
		
	}
}
/*
	this is the SIGCHILD handler dealing with mainly with background processes
	when a pipe forground process is running  (also deal with background running)
	return required message and appropriately handle zombie processes
*/
void sigchild_background_pipe_handler(int signum, siginfo_t *sig, void *context) 
{	
	// here we judge if the SIGCHILD is from current foreground child then we do not print
	if (in(sig->si_pid,pidList, pidList_size))
	{
	}
	else
	{
		size_t status;
		char* name = get_process_name_by_pid((sig->si_pid));
		// register return background process in order to collect them after handler returns
		if (return_background_size == 0)
		{
			return_background_pid_list = (pid_t *)malloc(sizeof(pid_t)*10);
		}
		else if (return_background_size > 0 && return_background_size % 10 == 0)
		{
			return_background_pid_list = (pid_t *)realloc(return_background_pid_list,sizeof(pid_t)*(return_background_size/10 + 1));
		}
		
		return_background_pid_list[return_background_size] = sig->si_pid;
		return_background_size++;
	}
}

/*
	this is the SIGUSR1 handler 
	doing nothing but help to trigger child process to execute new command
*/
void sigusr1_handler (int signum) 
{
	// printf("come to handle usr1\n");
	// here we do nothing but return
}

/* 
	this is the SIGCHILD handler when a non-pipe timeX instruction running
	record the time when getting this sigal and subtract with record starting time to get real time.
	get user time and system time from siginfo_t
*/
void sigchild_timeX_handler(int signum, siginfo_t *sig, void *context)
{

	struct timeval tv_1;
	gettimeofday(&tv_1, NULL);
	unsigned long long end_time =
	  (unsigned long long)(tv_1.tv_sec) * 1000 + (unsigned long long)(tv_1.tv_usec) / 1000;
	int time_elapse = end_time - (start_time);
	time_elapse = time_elapse / 10;
	int status;
	if (sig->si_pid == foreground_pid)
	{
		char* name = get_process_name_by_pid((sig->si_pid));
		printf("\n");
		printf("PID\tCMD\t\tRTIME\t\tUTIME\t\tSTIME\n");
		printf("%d\t%s\t\t%s\t\t%s\t\t%s\n", (sig->si_pid), name, prepare_time((int)time_elapse), prepare_time(sig->si_utime), prepare_time(sig->si_stime));
		waitpid(sig->si_pid, &status, 0);
	}
	else
	{
		size_t status;
		char* name = get_process_name_by_pid((sig->si_pid));
		printf("[%d] %s Done\n", (int)(sig->si_pid), name);
		waitpid ((int)(sig->si_pid), &status, 0);
	}
}

/* 
	this is the SIGCHILD handler when a pipe timeX instruction running
	record the time when getting this sigal and subtract with record starting time to get real time.
	get user time and system time from siginfo_t
*/
void sigchild_timeX_pipe_handler(int signum, siginfo_t *sig, void *context)
{

	struct timeval tv_1;
	gettimeofday(&tv_1, NULL);
	unsigned long long end_time =
	  (unsigned long long)(tv_1.tv_sec) * 1000 + (unsigned long long)(tv_1.tv_usec) / 1000;
	
	int status;
	if (in(sig->si_pid,pidList, pidList_size))
	{
		char* name = get_process_name_by_pid((sig->si_pid));
		int time_elapse = end_time - (start_time_pipe[get_index(pidList, pidList_size, sig->si_pid)]);
		time_elapse = time_elapse / 10;

		printf("\n");
		printf("PID\tCMD\t\tRTIME\t\tUTIME\t\tSTIME\n");
		printf("%d\t%s\t\t%s\t\t%s\t\t%s\n", (sig->si_pid), name,prepare_time((int)time_elapse), prepare_time(sig->si_utime), prepare_time(sig->si_stime));
		waitpid(sig->si_pid, &status, 0);
	}
	else
	{
		size_t status;
		char* name = get_process_name_by_pid((sig->si_pid));
		printf("[%d] %s Done\n", (int)(sig->si_pid), name);
		waitpid ((int)(sig->si_pid), &status, 0);
	}
}
