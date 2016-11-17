/*
Student name and No.: 	WANG Haicheng
Student  No.:			3035140108
Development platform:	Ubuntu 14.04
Last modified date:		Oct. 21
Compile: "make"
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include "define.h"
#include "sigHandler.h"
#include "parser.h"
#include "background_check_modify.h"



/*  
	this function is the interface to interact with user
	it maintains the prompt "## myshell % " interface, accept input with proper buffer size,
	pass the input to parser and get back organized instruction and option flag which are 
	dealing with special options like background or foreground and other special wrong case
	like wrong case about timeX and pipe
	parameters: instruction list struct to record organized instruction, char buffer accepting
		input, print_promt to decide whether to print promt token
	return: flag representing the mode (background or forground or other error)
*/
size_t readInstruction(InstructionList* listInstruction, char* buffer, bool print_prompt){
	struct sigaction sa;
	sigaction(SIGINT, NULL, &sa);
	sa.sa_handler = sigint_handler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGCHLD, NULL, &sa);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_handler = sigchild_background_handler;
	sigaction(SIGCHLD, &sa, NULL);
  	if (print_prompt)
  	{
    	printf("## myshell $ ");
  	}
  	else
  	{
  		print_prompt = true;
  	}
    memset( buffer, 0, BUFFERSIZE);
	fgets (buffer, BUFFERSIZE, stdin);
	size_t flag = 0;
	flag = parseInstruction(listInstruction, buffer);
	return flag;
}

/*
	this function is responsible for  executing valid instructions after every time interface 
	finish a loop to get instructions. 
	based on the foreground or background flag and distinguish the timeX function and non-timeX
	it will manipulate corresponsing singal handler setting to perform reqired output
	parameters: list instructions as valid instruction, flag representing the mode 
		(background or forground or other error)
	return: no return
*/

void executeUsingChild(InstructionList* listInstruction, int flag)
{	
	//register sigusr1 
		struct sigaction sa;
		sigaction(SIGUSR1, NULL, &sa);
		sa.sa_handler = sigusr1_handler;
		sigaction(SIGUSR1, &sa, NULL);
	// prepare for the child receiving SIGUSR1 (both pipe and non-pipe instructions)
		sigset_t myset;
		sigfillset(&myset);
		sigdelset(&myset,SIGUSR1);

	// handle single instruction without pipe
	if (listInstruction->instructionSize == 0)
	{
	// here we set up the sigchild handler accourding to the timeX cmd or not
		if (strcmp("timeX", listInstruction->list[0]->fileName) == 0)
		{
			sigaction(SIGCHLD, NULL, &sa);
			sa.sa_handler = sigchild_timeX_handler;
			sa.sa_flags = SA_SIGINFO | SA_RESTART;
			sigaction(SIGCHLD, &sa, NULL);
		}
		else
		{	
			sigaction(SIGCHLD, NULL, &sa);
			sa.sa_handler = sigchild_foreground_running_background_handler;
			sa.sa_flags = SA_SIGINFO | SA_RESTART;
			sigaction(SIGCHLD, &sa, NULL);
		}

		// check background or not to provide different actions
		if (flag == ISBACKGROUND)
		{
			listInstruction = removeBackgroundToken(listInstruction);
		}

		Instruction * instruction = listInstruction->list[0];
		pid_t pid = fork();
		if (pid == 0)
		{
			// child process
			// block the process until got the sigusr1 signal
    		sigsuspend(&myset);
				
			//register sigint handler
			sigaction(SIGINT, NULL, &sa);
			sa.sa_handler = sigusr1_handler;
			sigaction(SIGINT, &sa, NULL);
			int i;
			
			// distinguish the special timeX command or not
			if (strcmp("timeX", instruction->fileName) == 0)
			{
				i = execvp(instruction->arg[1], (instruction->arg+1));
			}
			else
			{
				i = execvp(instruction->fileName, instruction->arg);
			}

			if (i == -1)
			{
				fprintf(stderr, "myshell: '%s': No such file or directory\n", instruction->fileName);
				exit(-1);
			}
		}
		else
		{
			// parent process
			//send sigusr1 signal to let child execute 
			usleep(1000);
			kill(pid, SIGUSR1);
			if (strcmp("timeX", instruction->fileName) == 0)
			{
				if (start_time == 0)
				{
					start_time = 0;
				}
				struct timeval tv;
				gettimeofday(&tv, NULL);
				start_time = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
			}
			
			size_t status;

			// distinguish the background or forgound mode
			if (flag == NOBACKGROUND)
			{
				if (foreground_pid != 0)
				{
					foreground_pid = 0;
				}
				foreground_pid = pid;

				// ignore the SIGINT when foreground process is running
				struct sigaction sa;
				sigaction(SIGINT, NULL, &sa);
				sa.sa_handler = SIG_IGN;
				sigaction(SIGINT, &sa, NULL);

				// here we wait foreground process to finish
				pid_t child_pid;
				siginfo_t info;
				if (strcmp("timeX", instruction->fileName) == 0)
				{
					child_pid = waitid(P_PID, pid, &info, WNOWAIT | WSTOPPED | WCONTINUED);
				}
				else
				{	
					child_pid = waitpid(pid, &status, 0);
				}

				// here we use waitid with WNOHANG and waitpid again to clean and print out
				// the result of return background child when forground is running
				int itr = 0;
				int status;
				for(itr ; itr < return_background_size; itr ++)
				{
					char* name = get_process_name_by_pid(return_background_pid_list[itr]);
					printf("[%d] %s Done\n", return_background_pid_list[itr], name);
					waitpid(return_background_pid_list[itr], &status, 0);
				}
				if (return_background_size != 0)
				{
					free(return_background_pid_list);
					return_background_size = 0;
				}

				// set myshell aware of the SIGINT again after foreground finish
				sigaction(SIGINT, NULL, &sa);
				sa.sa_handler = sigint_handler;
				sigaction(SIGINT, &sa, NULL);
			}
			else if (flag == ISBACKGROUND)
			{
				// set back the SIGINT handler
				struct sigaction sa;
				sigaction(SIGINT, NULL, &sa);
				sa.sa_handler = sigint_handler;
				sigaction(SIGINT, &sa, NULL);
				// set the SIGCHILD handler
				sigaction(SIGCHLD, NULL, &sa);
				sa.sa_flags = SA_SIGINFO;
				sa.sa_handler = sigchild_background_handler;
				sigaction(SIGCHLD, &sa, NULL);
				
			
				 // change group id to avoid SIGINT
				setpgid(pid, pid);				
			}
		}
	}

	// handle pipe 
	else
	{
		// set the SIGCHILD handler to handle timeX
		if (strcmp("timeX", listInstruction->list[0]->fileName) == 0)
		{
			sigaction(SIGCHLD, NULL, &sa);
			sa.sa_flags = SA_SIGINFO | SA_RESTART;
			sa.sa_handler = sigchild_timeX_pipe_handler;
			sigaction(SIGCHLD, &sa, NULL);
		}
		else
		{
			sigaction(SIGCHLD, NULL, &sa);
			sa.sa_handler = sigchild_background_pipe_handler;
			sa.sa_flags = SA_SIGINFO | SA_RESTART;
			sigaction(SIGCHLD, &sa, NULL);
		}

		// if it is background we remove the background token to execute
		if (flag == ISBACKGROUND)
		{
			listInstruction = removeBackgroundToken(listInstruction);
		}

		// maintain global variables to avoid memory leakage
		if (pidList != NULL)
		{
			free(pidList);
			pidList_size = 0;
		}
		if (start_time_pipe != NULL)
		{
			free(start_time_pipe);
			pidList_size = 0;
		}
		pidList = (pid_t*)malloc(sizeof(pid_t)*(listInstruction->instructionSize+1));
		pidList_size = listInstruction->instructionSize+1;
		start_time_pipe = (unsigned long long*)malloc(sizeof(unsigned long long)*pidList_size);
		pid_t currentPid;

		// create pipe
		int ** pfd = (int**)malloc(sizeof(int*)*listInstruction->instructionSize);
		int i;
		for (i = 0; i < listInstruction->instructionSize; i++)
		{
			pfd[i] = (int*)malloc(sizeof(int)*2);
			pipe(pfd[i]);
		}

		// loop to fork child processes
		for (i = 0; i < listInstruction->instructionSize + 1; i++)
		{
			currentPid = fork();
			if (currentPid == 0) 
			{ 	
				//every child
				// block the process until got the sigusr1 signal
    			sigsuspend(&myset);
				// setting for corresponding close and dup2 pipe
				int j;
				if (i == 0){
					close(pfd[i][0]);

					for (j = 1; j < listInstruction->instructionSize; j++)
					{
						close(pfd[j][0]);
						close(pfd[j][1]);

					}
					dup2(pfd[i][1], 1);
				}
				else if (i == listInstruction->instructionSize)
				{
					close(pfd[i-1][1]);

					for (j = 0; j < listInstruction->instructionSize-1; j++)
					{
						close(pfd[j][0]);
						close(pfd[j][1]);

					}
					dup2(pfd[i-1][0], 0);

				}
				else
				{
					close(pfd[i-1][1]);  
					close(pfd[i][0]);

					for (j = 0; j < listInstruction->instructionSize; j++)
					{
						if (j!= i && j != i-1)
						{
							close(pfd[j][0]);
							close(pfd[j][1]);
						}
						
					} 
					dup2(pfd[i-1][0], 0);
					dup2(pfd[i][1], 1); 

				}
				int check;
				// handle timeX special command
				if (i == 0 && strcmp("timeX", listInstruction->list[0]->fileName) == 0)
				{
					 check = execvp(listInstruction->list[i]->arg[1], listInstruction->list[i]->arg+1);
				}
				else
				{
					check = execvp(listInstruction->list[i]->fileName, listInstruction->list[i]->arg);
				}
				if (check == -1)
				{
					fprintf(stderr, "myshell: Fail to execute '%s': No such file or directory\n", listInstruction->list[i]->fileName);
					exit(-1);
				}
			}
			else
			{
				pidList[i] = currentPid;

				//send sigusr1 signal to let child execute 
				usleep(5000);
				kill(currentPid, SIGUSR1);

				// record time
				struct timeval tv;
				gettimeofday(&tv, NULL);
				start_time_pipe[i] = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
			}	
		}

		// Here is the parent process manipulation
		// firstly close all pipe to avoid forever waiting
		int j;
		for (j = 0; j < listInstruction->instructionSize; j++)
		{
			close(pfd[j][0]);
			close(pfd[j][1]);
		}
		if (flag == NOBACKGROUND)
		{
			// ignore the SIGINT when foreground process is running
			struct sigaction sa;
			sigaction(SIGINT, NULL, &sa);
			sa.sa_handler = SIG_IGN;
			sigaction(SIGINT, &sa, NULL);

			// block other SIGCHILD signal
			sigset_t blockMask;
			if (strcmp("timeX", listInstruction->list[0]->fileName) != 0)
			{				
				
			}

			// here we wait foreground process to finish
			pid_t child_pid;
			size_t status;
			if (strcmp("timeX", listInstruction->list[0]->fileName) == 0)
			{
				int info;
				int k = 0;
				// wait twice for more stable performance with almost no cost when timeX in pipe
				for (k; k < 2; k++){
					for (j = 0; j < listInstruction->instructionSize + 1;j++)
					{
						child_pid = waitid(P_PID, pidList[j], &info, WNOWAIT |  WCONTINUED | WEXITED);
					}
				}				
			}
			else
			{
				for (j = 0; j < listInstruction->instructionSize + 1;j++)
				{
					child_pid = waitpid(pidList[j], &status, 0);
				}
			}
			// here collect all the return background process when foreground pipe processes running
			int itr = 0;
			int status_1;
			for(itr ; itr < return_background_size; itr ++)
			{
				char* name = get_process_name_by_pid(return_background_pid_list[itr]);
				printf("[%d] %s Done\n", return_background_pid_list[itr], name);
				waitpid(return_background_pid_list[itr], &status_1, 0);
			}
			if (return_background_size != 0)
			{
				free(return_background_pid_list);
				return_background_size = 0;
			}
			
			// set myshell aware of the SIGINT again after foreground finish
			sigaction(SIGINT, NULL, &sa);
			sa.sa_handler = sigint_handler;
			sigaction(SIGINT, &sa, NULL);

		}
		else
		{
			// set the SIGINT handler
			struct sigaction sa;
			sigaction(SIGINT, NULL, &sa);
			sa.sa_handler = sigint_handler;
			sigaction(SIGINT, &sa, NULL);

			// set the SIGCHILD handler
			sigaction(SIGCHLD, NULL, &sa);
			sa.sa_flags = SA_SIGINFO;
			sa.sa_handler = sigchild_background_handler;
			sigaction(SIGCHLD, &sa, NULL);
			
			// change group id to avoid later SIGINT impact
			pid_t group_id = getpgrp();
			group_id ++; 
			int j; 
			for (j = 0; j < listInstruction->instructionSize + 1;j++)
			{
				setpgid(pidList[j], group_id);	
			}
		}
	}
}

/*
	main function
	responsible for initialize the program
	one more feature is that some hard code errors showing are listing here
*/
int main()
{ 
	char* buffer = (char*) malloc(sizeof(char)*BUFFERSIZE);

	InstructionList* listInstruction = (InstructionList* ) malloc(sizeof(InstructionList));
	bool print_prompt = true;
	while (1)
	{
		listInstruction->list = NULL;
		listInstruction->instructionSize = 0;
		size_t flag = readInstruction(listInstruction, buffer, print_prompt);
		print_prompt = true;

		// the following are some special error
		if (flag == EXIT)
		{
			printf("myshell: Terminated\n");
			break;
		}
		else if (flag == EXIT_TOO_MANY_ARGUMENTS)
		{
			fprintf(stderr, "myshell: \"exit\" with other arguments!!!\n" );
		}
		else if (flag == WRONGBACKGROUND)
		{
			fprintf(stderr, "myshell: '&' should not appear in the middle of the command line\n" );
		}
		else if (flag == GETSIGCHILD)
		{
			// printf("\n");
			print_prompt = false;
			continue;
		}
		else if (flag == CONTINUEWITHOUTEXECUTE)
		{
			continue;

		}
		else if (flag == INCOMLETEPIPE)
		{
			fprintf(stderr, "myshell: Incomplete '|' sequence\n" );
		}
		else if (flag == TIMEXBACKGROUND)
		{
			fprintf(stderr, "myshell: \"timeX\" cannot be run in background mode\n" );
		}
		else if (flag == TIMEXSTANDALONE)
		{
			fprintf(stderr, "myshell: \"timeX\" cannot be a standalone command\n" );
		}
		else if (flag == TIMEXINCOMLETEPIPE)
		{
			fprintf(stderr, "myshell: Cannot timeX an incomplete '|' sequence\n" );
		}

		// if no error then execute the valid instruction
		else
		{
			executeUsingChild(listInstruction, flag);
		}

	}
}