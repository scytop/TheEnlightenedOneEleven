// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <error.h>
#include "stack.h"

#define DEFAULT_BUFFER_SIZE 50
/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

struct node{
	struct node *next;
	command_t command;
};
unsigned int i=0;
/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

struct command_stream{
	struct node *start;
};

void destroyBeginSpaces(char * input){
	int spacesToMove = 0;
	for(i = 0; i < strlen(input); i++)
	{
		if (input[i] == ' ')
			spacesToMove++;
		else
			break;
	}
	memmove(input, input+spacesToMove, strlen(input));
}

void destroyEndSpaces(char * input){
	for(i = strlen(input)-1; i > 0; i--)
	{
		if(input[i] == ' ')
			input[i] = '\0';
		else
			return;
	}
}

bool isOperand(char c)
{
	if(c=='|' || c== '&' || c == ';' || c== '(' 
		|| c == ')')
		return true;
	else
		return false;
}
bool isSimpleCommand(char c)
{
	if(isalnum(c) || c == ' ')
		return true;
	return false;
}

char ** lexer (int(*get_next_byte) (void *), 
					void *get_next_byte_argument,
					int * arraySize) 
{
char c;
char prev_c = '\0';
char not = '\0';
char* nullpoint = &not;

char * currentString = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
char ** stringArray = malloc(sizeof(char*) * DEFAULT_BUFFER_SIZE*10);
int currentPos = 0;
unsigned int maxArrayElem = 0;

while(( c = get_next_byte(get_next_byte_argument)) &&( c != EOF))
	{
		//assume that only valid inputs are allowed
		if (c == '(' || c == ')' || c == ';')
		{
		//These are singular operands, always, so this should push
		//and create a new cstring
		strcat(currentString, &c);
		strcat(currentString, nullpoint);
		stringArray[maxArrayElem] = currentString;
		currentString = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
		currentPos = 0;
		maxArrayElem++;
		prev_c = '\0'; //kind of hacky, but ensures the next iteration will
									//be an append instead of a string-push
		}
		else if(prev_c == '\0' || 
					(isOperand(c) && isOperand(prev_c))
			)
			{ //if the current string needs to be appended
				//FIXME: implement what happens if they go over 500 chars
				strcat(currentString, &c);
				currentPos++;
				prev_c = c;

			}
		else if(isOperand(c) != isOperand(prev_c))
		{ //if we need to push the current string to the stringArray
			strcat(currentString, nullpoint);
			stringArray[maxArrayElem] = currentString; //pushes c string
			currentString = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
				//creates a new c string 
			currentPos = 0;
			strcat(currentString, &c);
			maxArrayElem++;
			prev_c = c;
		}
	}
//at EOF, push current string onto the array
strcat(currentString, nullpoint);
stringArray[maxArrayElem] = currentString;
maxArrayElem++;
currentString = malloc(sizeof(char)*2);
currentString[0] = '\0';
stringArray[maxArrayElem] = currentString;
for(i = 0; i < maxArrayElem; i++)
	{
	destroyBeginSpaces(stringArray[i]);
	destroyEndSpaces(stringArray[i]);
	}

*arraySize = maxArrayElem;
return stringArray;

}



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

command_stream_t parseShitPls(char **stringArray, int arrSize){
	//initialize operator and command stacks
	struct stack opStack;
	opStack.commands = malloc(sizeof(command_t)*5);
	opStack.size = 0;
	opStack.max_size = 5;
	stack comStack;
	comStack.commands = malloc(sizeof(command_t)*5);
	comStack.size = 0;
	comStack.max_size = 5;

	//make node + pointer to current node
	node *first = malloc(sizeof(node));
	first->next = NULL;
	node *curNode;
	curNode = first;

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

	return first;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
	int * tmpPnt;
	char ** commandArray = lexer((*get_next_byte) (void *), 
													*get_next_byte_argument,
													tmpPnt);
	int arraySize = *tmpPnt;
	//returns an array of all the commands
	
	return parseShitPls(commandArray, arraySize);
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  //error (1, 0, "command reading not yet implemented");
  //return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
