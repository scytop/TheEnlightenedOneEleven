struct node{
	node *next;
	command_t command;
};

command_t makeCommand(char *curString, int type){
	command_t result = malloc(sizeof(command));

	//set type of command
	switch(type){
		case 0: 
			result->type = SIMPLE_COMMAND;
			break;
		case 1:
			result->type = AND_COMMAND;
			break;
		case 2: 
			result->type = OR_COMMAND;
			break;
		case 3:
			result->type = PIPE_COMMAND;
			break;
		case 4:
		case 5:
			result->type = SEQUENCE_COMMAND;
			break;
		case 6:
			result->type = SUBSHELL_COMMAND;
			break;
	}

	//set pointer to command if simple command
	if(type == 0){
		char *comString = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
		strcat(comString, curString);
		result->u.word = &comString;
	}

	return result;
}

//for all commands besides simple commands
command_t combineCommand(command_t command1, command_t command2, command_t operator){
	//send in same command twice to indicate subshell operator
	if(command1 == command2)
		operator->u.subshell_command = &command1;
	else{
		operator->u.command[0] = command1;
		operator->u.command[1] = command2;
	}
	return operator;
}

int precedence(command_t command){
	if(command->type == SEQUENCE_COMMAND)
		return 0;
	else if(command->type == PIPE_COMMAND)
		return 2;
	else if(command->type == SUBSHELL_COMMAND)
		return 1;
	else //either AND or OR command
		return 3;
}

void parseShitPls(char **stringArray, int arrSize){
	//initialize operator and command stacks
	stack opStack;
	opStack.commands = malloc(sizeof(command_t)*5);
	opStack.size = 0;
	opStack.max_size = 5;
	stack comStack;
	comStack.commands = malloc(sizeof(command_t)*5);
	comStack.size = 0;
	comStack.max_size = 5;

	//make node + pointer to current node
	node first;
	first.next = NULL;
	node *curNode;
	curNode = &first;

	//go through entire array
	for(int k=0 ; k < arrSize ; k++){
		int comType = 0;
		if(strcmp(stringArray[k], "&&") == 0)
			comType = 1;
		if(strcmp(stringArray[k], "||") == 0)
			comType = 2;
		if(strcmp(stringArray[k], "|") == 0)
			comType = 3;
		if(strcmp(stringArray[k], ";") == 0)
			comType = 4;
		if(strcmp(stringArray[k], "\n") == 0)
			comType = 5;
		if(strcmp(stringArray[k], "(") == 0)
			comType = 6;

		//all the stack popping stuff that comes with close paren
		if(strcmp(stringArray[k], ")") == 0){
			command_t topOp = opStack.pop();
			while(topOp->type != SUBSHELL_COMMAND){
				command_t command2 = comStack.pop();
				command_t command1 = comStack.pop();
				command_t newCommand = combineCommand(command1, command2, topOp);
				comStack.push(newCommand);
				topOp = opStack.pop();
			}
			command_t command1 = comStack.pop();
			command_t newCommand = combineCommand(command1, command1, topOp);
			comStack.push(newCommand);
			continue;
		}

		//we have at least two newlines, make new node
		if(stringArray[k][0] == '\n' && stringArray[k][1] == '\n'){
			while(opStack.peek() != NULL){
				command_t operator = opStack.pop();
				command_t command2 = comStack.pop();
				command_t command1 = comStack.pop();
				command_t newCommand = combineCommand(command1, command2, operator);
				comStack.push(newCommand);
			}
			node *newNode = malloc(sizeof(node));
			newNode->next = NULL;
			curNode->next = newNode;
			curNode->command = comStack.pop();
			curNode = newNode;
			continue;
		}

		command_t curCommand = makeCommand(stringArray[k], comType);
		if(curCommand->type == SIMPLE_COMMAND) //not an operator
			comStack.push(curCommand);
		else if(curCommand->type == SUBSHELL_COMMAND) //encounter open paren
			opStack.push(curCommand);
		else{
			if(opStack.peek() == NULL)
				opStack.push(curCommand);
			else{
				if(precedence(curCommand) > precedence(opStack.peek()))
					opStack.push(curCommand);
				else{
					command_t operator = NULL;
					while(operator->type != SUBSHELL_COMMAND && precedence(operator) <= precedence(opStack.peek())){
						operator = opStack.pop();
						command_t command2 = comStack.pop();
						command_t command1 = comStack.pop();
						command_t newCommand = combineCommand(command1, command2, operator);
						comStack.push(newCommand);
						if(opStack.peek() == NULL)
							break;
					}
					opStack.push(curCommand);
				}
			}
		}
	}
	//pop off everything left at the end, should not have any parens/newlines on the stack by now
	//[if there is I fucked up]
	while(opStack.peek() != NULL){
		command_t operator = opStack.pop();
		command_t command2 = comStack.pop();
		command_t command1 = comStack.pop();
		command_t newCommand = combineCommand(command1, command2, operator);
		comStack.push(newCommand);
	}

	curNode->command = comStack.pop();
}