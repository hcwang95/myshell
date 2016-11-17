/*
	this function is to check instrucitons imply running in backgound mode
	or forground.
	parameter: list of instrucitons as an organized instrctions struct
	return: predefined int as different mode or wrong background
*/
int checkBackgound(InstructionList* listInstruction)
{
	// first case of background like "emacs&"
	if (listInstruction->list[0]->fileName[strlen(listInstruction->list[0]->fileName)-1] == BACKGROUND
		&& listInstruction->list[0]->argSize == 1
		&& listInstruction->instructionSize == 0)
	{
		return ISBACKGROUND;
	}
	int check = 0;
	int j =0; 
	for (j; j < listInstruction->instructionSize + 1; j++)
	{
		int i = 0;
		for (i; i< listInstruction->list[j]->argSize; i++)
		{
			if (listInstruction->list[j]->arg[i][strlen(listInstruction->list[j]->arg[i])-1] == BACKGROUND )
			{
				check ++;
			}
		}
	}
	// second case of background like "emacs &" with whitespace between
	if (!check)
	{
		return NOBACKGROUND;
	}
	else if(check == 1)
	{

		if (listInstruction->list[j-1]->arg[listInstruction->list[j-1]->argSize - 1]
			[strlen(listInstruction->list[j-1]->arg[listInstruction->list[j-1]->argSize - 1])-1] == BACKGROUND)
		{
			return ISBACKGROUND;
		}
		else
		{

			return WRONGBACKGROUND;
		}
	}
	else
	{
		return WRONGBACKGROUND;
	}
}

/*
	this function is to remove backgound token '&' when executing the instuction
	parameter: list of instrucitons as an organized instrctions struct
	return: list of instrucitons as an organized instrctions without & token
*/

InstructionList* removeBackgroundToken(InstructionList* listInstruction)
{
	// handle "emacs&" and "cat ## | wc&"
	if (listInstruction->list[listInstruction->instructionSize]->fileName[strlen(listInstruction->list[listInstruction->instructionSize]->fileName) -1 ] == BACKGROUND)
	{
		listInstruction->list[listInstruction->instructionSize]->fileName[strlen(listInstruction->list[listInstruction->instructionSize]->fileName) -1 ] = 0;
		listInstruction->list[listInstruction->instructionSize]->arg[0][strlen(listInstruction->list[listInstruction->instructionSize]->arg[0]) - 1] = 0;
	}
	// handle "ls -l&" or "cat ## | wc -l&"
	else 
	{
		if (strlen(listInstruction->list[listInstruction->instructionSize]->arg[listInstruction->list[listInstruction->instructionSize]->argSize - 1]) != 1)
		{
			listInstruction->list[listInstruction->instructionSize]->arg[listInstruction->list[listInstruction->instructionSize]->argSize - 1]
			[strlen(listInstruction->list[listInstruction->instructionSize]->arg[listInstruction->list[listInstruction->instructionSize]->argSize - 1]) - 1] = 0;
		}
		// handle "ls &" or "cat ## | wc -l &"
		else 
		{
			listInstruction->list[listInstruction->instructionSize]->arg[listInstruction->list[listInstruction->instructionSize]->argSize - 1] = 0 ;	
			listInstruction->list[listInstruction->instructionSize]->argSize --;			
		}
	}
	return listInstruction;
}