/*
Student name and No.: 	WANG Haicheng
Student  No.:			3035140108
Development platform:	Ubuntu 14.04
Last modified date:		Oct. 21
Compile: "make"
*/

void debug(InstructionList* listInstruction)
{
	printf("here is the parsing results\n");
	size_t i = 0;
	for (i; i<listInstruction->instructionSize+1; i++)
	{
		printf("the %d-th instruction\n", i+1);
		printf("fileName is *%s*\n", listInstruction->list[i]->fileName);
		printf("total %d argments:\n", listInstruction->list[i]->argSize);
		size_t j = 0;
		for (j = 0; j < listInstruction->list[i]->argSize; j++)
		{
			printf("*%s*\n", listInstruction->list[i]->arg[j]);
		}
	}
	
}
/* 
	this function is helper function to deep copy an char array
	parameter: char array as source array we want to copy
	return: a new char array object that need to be pass to an char* variable.
*/
char* deepCopy(char* source)
{
	char* target = NULL;
	target = (char* )malloc( (int)strlen(source) + 1);
	size_t i;
	for (i = 0; source[i] !='\0'; i++ )
	{
		target[i] = source[i];
	}
	return target;
}

/*
	main parser function
	responsiable to accept an string as input and then parse them into structurize instructions
	and be able to distinguish valid and non-valid instruction and find out what is the error type
	or what is the running type (foreground or background) using checkgorund function
	parameter: list of instructions that store the structurize instructions, char array as raw input.
	return: int as predefined mode, determine valid or not and what valid mode or what error type.
*/
size_t parseInstruction(InstructionList* listInstruction, char* buffer)
{
	char* secondBuffer = deepCopy(buffer);
	char * token;
	const char deliminator = WHITESPACE;
	int numOfInstrcution = 1;
	// here if we got buffer without any thing
	// then it must be the case that we encounter the SIGINT
	// here we check it first and then can handle the SIGINT
	if (buffer[0] == 0)
	{
		return GETSIGCHILD;
	}
	// here we go through once to get the number of the pipe 
	token = strtok(buffer, &deliminator);
	while (token){
		token[strcspn(token, "\n")] = 0;
		if (strcmp("|",token) == 0){
			numOfInstrcution++;	
			
		}
		token = strtok(NULL,  &deliminator);
	}
	listInstruction->list = (Instruction**)malloc(sizeof(Instruction*)*numOfInstrcution);
	listInstruction->instructionSize = 0;

	// prepare for the first instruction
	listInstruction->list[listInstruction->instructionSize] = (Instruction*)malloc(sizeof(Instruction));
	bool first = true;
	listInstruction->list[listInstruction->instructionSize]->argSize = 0;
	size_t i = 0;
	listInstruction->list[listInstruction->instructionSize]->arg = (char **)malloc(sizeof(char*) * 30);
	for (i; i < 30; i++)
	{
		listInstruction->list[listInstruction->instructionSize]->arg[i] = NULL;
	}
	// loop the instruction second time to do the real parsing
	token = strtok(secondBuffer, &deliminator);
	while (token)
	{
		token[strcspn(token, "\n")] = 0;
		if (strlen(token) != 0)
		{
			// handle the case "ps | wc"
			if (strcmp("|",token) == 0)
			{
				listInstruction->instructionSize++;
				listInstruction->list[listInstruction->instructionSize] = (Instruction*)malloc(sizeof(Instruction));;
				first = true;
				i = 0;
				listInstruction->list[listInstruction->instructionSize]->arg = (char **)malloc(sizeof(char*) * 30);
				for (i; i < 30; i++)
				{
					listInstruction->list[listInstruction->instructionSize]->arg[i] = NULL;
				}
			}
			// handle the case "ps |wc" where there is no white space on the right of '|'
			else if(token[0] == '|')
			{
				listInstruction->instructionSize++;
				listInstruction->list[listInstruction->instructionSize] = (Instruction*)malloc(sizeof(Instruction));;
				i = 0;
				listInstruction->list[listInstruction->instructionSize]->arg = (char **)malloc(sizeof(char*) * 30);
				for (i; i < 30; i++)
				{
					listInstruction->list[listInstruction->instructionSize]->arg[i] = NULL;
				}
				if (token[strlen(token)-1] != '|')
				{
					listInstruction->list[listInstruction->instructionSize]->fileName = deepCopy(token+1);
					listInstruction->list[listInstruction->instructionSize]->arg[listInstruction->list[listInstruction->instructionSize]->argSize] = deepCopy(token+1);
					listInstruction->list[listInstruction->instructionSize]->argSize ++;
					first = false;
				}
				else
				// handle the case " ps |wc| wc"
				{
					token[strlen(token)-1] = 0;
					listInstruction->list[listInstruction->instructionSize]->fileName = deepCopy(token+1);
					listInstruction->list[listInstruction->instructionSize]->arg[listInstruction->list[listInstruction->instructionSize]->argSize] = deepCopy(token+1);
					listInstruction->list[listInstruction->instructionSize]->argSize ++;
					listInstruction->instructionSize++;
					listInstruction->list[listInstruction->instructionSize] = (Instruction*)malloc(sizeof(Instruction));;
					i = 0;
					listInstruction->list[listInstruction->instructionSize]->arg = (char **)malloc(sizeof(char*) * 30);
					for (i; i < 30; i++)
					{
						listInstruction->list[listInstruction->instructionSize]->arg[i] = NULL;
					}
					first = true;
				}
				

			}
			// handle the case "cat | grep ###| wc" where there is no white space on the left of '|'
			else if (token[strlen(token)-1] == '|')
			{
				token[strlen(token)-1] = 0;
				if (listInstruction->list[listInstruction->instructionSize]->fileName == NULL)
				{
					listInstruction->list[listInstruction->instructionSize]->fileName = deepCopy(token);
				}
				listInstruction->list[listInstruction->instructionSize]->arg[listInstruction->list[listInstruction->instructionSize]->argSize] = deepCopy(token);
				listInstruction->list[listInstruction->instructionSize]->argSize ++;
				listInstruction->instructionSize++;
				listInstruction->list[listInstruction->instructionSize] = (Instruction*)malloc(sizeof(Instruction));
				first = true;
				i = 0;
				listInstruction->list[listInstruction->instructionSize]->arg = (char **)malloc(sizeof(char*) * 30);
				for (i; i < 30; i++)
				{
					listInstruction->list[listInstruction->instructionSize]->arg[i] = NULL;
				}
			}
			// if no pipe token we just do normal structurizing
			else 
			{
				if (first)
				{
					listInstruction->list[listInstruction->instructionSize]->fileName = deepCopy(token);
					listInstruction->list[listInstruction->instructionSize]->arg[listInstruction->list[listInstruction->instructionSize]->argSize] = deepCopy(token);
					listInstruction->list[listInstruction->instructionSize]->argSize ++;
					first = false;
				}
				else
				{
					listInstruction->list[listInstruction->instructionSize]->arg[listInstruction->list[listInstruction->instructionSize]->argSize] = deepCopy(token);
					listInstruction->list[listInstruction->instructionSize]->argSize ++;
				}
			}
			
		}
		token = strtok(NULL,  &deliminator);
	}

	// the following are checking parsing result to get flag

	// this is to handle an empty enter
	if(listInstruction->list[0]->fileName == NULL || listInstruction->list[0]->fileName[0] == 0)
	{
		return CONTINUEWITHOUTEXECUTE;
	}
	// this is to handle valid exit
	else if (strcmp("exit",listInstruction->list[0]->fileName) == 0 && listInstruction->list[0]->arg[1] == 0 && listInstruction->instructionSize == 0) 
		
	{
		return EXIT;
	}
	// this is to handle invalid exit
	else if(strcmp("exit",listInstruction->list[0]->fileName) == 0 &&
			listInstruction->list[0]->argSize > 1
			|| (strstr(listInstruction->list[0]->fileName, "exit") &&
				strlen(listInstruction->list[0]->fileName))
			|| (strcmp("exit",listInstruction->list[0]->fileName) == 0 &&
				listInstruction->instructionSize>1)
			|| (strstr(listInstruction->list[0]->fileName, "exit") &&
				listInstruction->instructionSize>1))
	{
		return EXIT_TOO_MANY_ARGUMENTS;
	}
	// this is to handle pipe without enough instruction
	else if (listInstruction->list[listInstruction->instructionSize]->fileName == NULL)
	{
		return INCOMLETEPIPE;
	}
	// this is to handle wrong timeX instruction with background mode
	else if (strcmp("timeX", listInstruction->list[0]->fileName)==0 
		&& checkBackgound(listInstruction) == ISBACKGROUND)
	{
		return TIMEXBACKGROUND;
	}
	// this is to handle wrong standalone timeX instruction 
	else if (strcmp("timeX", listInstruction->list[0]->fileName)==0 
		&& listInstruction->instructionSize == 0
		&& listInstruction->list[0]->argSize == 1)
	{
		return TIMEXSTANDALONE;
	}
	// this is to handle valid timeX instruction 
	else if (strcmp("timeX", listInstruction->list[0]->fileName)==0 
		&& listInstruction->list[0]->argSize == 1
		&& listInstruction->instructionSize > 0)
	{
		return TIMEXINCOMLETEPIPE;
	}
	// if not fall into special mode above then check it is backgorund mode or foreground
	return checkBackgound(listInstruction);
}