// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <error.h>

#define DEFAULT_BUFFER_SIZE 300
/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */


struct stack
{
  command_t *commands;
  int size;
  int max_size;
  void (* push)();
  command_t (* pop)();
  command_t (* peek)();
};
 
void
push (struct stack *self, command_t command)
{
  if (self->size == self->max_size) {
    self->max_size *= 2;
    command_t *new = malloc (self->max_size * sizeof (command_t));
    self->commands = new;
  }
 
  self->commands[self->size] = command;
  self->size++;
}
 
command_t
pop (struct stack *self)
{
  if (self->size > 0)
    return self->commands[self->size--];
  else
    return NULL;
}
 
command_t
peek (struct stack *self)
{
  if (self->size > 0) {
    int i = self->size - 1;
    return self->commands[i];
  } else
    return NULL;
}
 
struct stack*
init_stack (int max)
{
  struct stack *new = malloc (sizeof (struct stack));
  new->size = 0;
  new->max_size = max;
  new->commands = malloc (max * sizeof (command_t));
	return new;
}


struct node{
	struct node *next;
	command_t command;
};
unsigned int i=0;
/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

struct command_stream{
	struct node *start;
	struct node *iterator;
};

void destroyBeginSpaces(char * input){
	int spacesToMove = 0;
	for(i = 0; i < strlen(input); i++)
	{
		if (input[i] == ' ' || input[i] == '\t')
			spacesToMove++;
		else
			break;
	}
	memmove(input, input+spacesToMove, strlen(input));
}

void destroyEndSpaces(char * input){
	for(i = strlen(input)-1; i > 0; i--)
	{
		if(input[i] == ' ' || input[i] == '\t')
			input[i] = '\0';
		else
			return;
	}
}
/*void yumCarrots(char * simp)
{
unsigned int i = 0;

//if the first char is a <, well... we're in trouble
//should have error'd in the checkDontShrek
for(i = 1; i < strlen(simp); i++)
{
if(simp[i] = '<' || simp[i] = '>')
	{
	
	}
}
*/
bool isOperand(char c)
{
	if(c=='|' || c== '&' || c == ';' || c== '(' 
		|| c == ')' || c== '\n'  /* || c == '<' || c == '>'*/)
		return true;
	else
		return false;
}

bool isSimpleCommand(char c)
{
	if(isalnum(c) || c == ' ' || c == '!' || c == '%'||
		c == '+' || c == ',' || c == '-' || c == '.' ||
		c == '/' || c == ':' || c == '@' || c == '^'|| c == '_')
		return true;
	return false;
}

bool swag(char c)
{
if(isOperand(c) || c== '<' || c == '>' || isSimpleCommand(c))
	return true;
else
	return false;
}

char ** lexer (int(*get_next_byte) (void *), 
					void *get_next_byte_argument,
					unsigned int * arraySize) 
{
char c;
char prev_c = '\0';
char not = '\0';
char* nullpoint = &not;

char * currentString = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
char ** stringArray = malloc(sizeof(char*) * DEFAULT_BUFFER_SIZE*10);
int currentPos = 0;
unsigned int maxArrayElem = 0;
bool prevIsCommand = false;
bool currIsCommand =false;
int openCount = 0;
int closeCount = 0;
bool isComment = false;


while(( c = get_next_byte(get_next_byte_argument)) &&( c != EOF))
	{
	if(c == '#'){isComment = true;continue;}
	if(c != '\n' && isComment) {continue;}
	if(isComment && c== '\n'){isComment = false;}
		prevIsCommand = isOperand(prev_c);
		currIsCommand = isOperand(c);
		//assume that only valid inputs are allowed
		if (c == '(' || c == ')' || c == ';')
		{
		//These are singular operands, always, so this should push
		//and create a new cstring
		if (c == '(')
			openCount++;
		if (c == ')')
			closeCount++;	
		

		strcat(currentString, &c);
		strcat(currentString, nullpoint);
		stringArray[maxArrayElem] = currentString;
		currentString = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
		currentPos = 0;
		maxArrayElem++;
		prev_c = '\0'; //kind of hacky, but ensures the next
				// iteration will
				//be an append instead of a string-push
		}
		else if(((isOperand(c) != isOperand(prev_c)) ||
			 (c != '\n' && prev_c == '\n')) ||
			((prev_c == '|' || prev_c == '&') && c == '\n')	)
		{ //if we need to push the current string to the stringArray
			destroyBeginSpaces(currentString);
			strcat(currentString, nullpoint);
			if(*currentString != '\0'){
			stringArray[maxArrayElem] = currentString;
				 //pushes c string
			currentString=malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
				//creates a new c string 
			currentPos = 0;
			maxArrayElem++;}
			strcat(currentString, &c);
			prev_c = c;
		}
		else if((prev_c == '\0') ||
					 (prevIsCommand==currIsCommand))
			{ //if the current string needs to be appended
	//FIXME: implement what happens if they go over 500 chars
				strcat(currentString, &c);
				currentPos++;
				prev_c = c;

			}
	}
//at EOF, push current string onto the array
if(prev_c == '\n')
	{currentString[currentPos] = '\0';}
if(openCount != closeCount)
	error(2,2, "Open Paren Count doesn't match Close Paren Count");
if(currentString[0] !=  '\0')
{
strcat(currentString, nullpoint);
stringArray[maxArrayElem] = currentString;
maxArrayElem++;
}
if(currIsCommand && prev_c != '\n')
{
error(3,3, "Operator is at the end D:");
}
unsigned int i = 0;
for(i = 0; i < maxArrayElem; i++)
	{
	destroyBeginSpaces(stringArray[i]);
	destroyEndSpaces(stringArray[i]);
	}
*arraySize = maxArrayElem;
return stringArray;


}
void checkDontShrek(char** array, unsigned int *arraySize){
	//run after lex-luthering
	unsigned int i = 0;
	unsigned int j = 0;
	char c = '\0';
	char prev_c = '\0';
	for(i = 0; i < *arraySize; i++) //for each token
	{
		for(j = 0; j < strlen(array[i]); j++)
			{
			c = array[i][j];
			//checks if "<<" or ">>" occurs
			if((c == prev_c) && (c== '<' || c == '>'))
				error(4,4,"2fast2furious");
			else if ((c == prev_c) && (c==';'))
				error(5,5,"prev_c"); //makes sure ";;" occurs
			else if((swag(c) || isSimpleCommand(c)) )
				{prev_c = c; continue;} //normal case
			else 
			//c isn't an "acceptable char", as seen in swag(c)
				error(c,c, "A non-thing is here!");		
			}
	if(isOperand(array[i][0]) && (strlen(array[i]) >=3)&&
		array[i][0] != '\n') 
		error(7,7,"2 many operands"); //3 operands in a row
	if(array[i][0] == '<' || array[i][0] == '>' || 
				//carrot at beginning or end
		array[i][strlen(array[i])-1] == '>' ||
		array[i][strlen(array[i])-1] == '<')
		error(8,8, "ksjgdlg");
	if(i == (*arraySize)-1 && isOperand(array[i][0]) && array[i][0]!='\n')
		error(9,9,"ERROR UP ");
	if(i > 0 && (array[i][0] == '|' || array[i][0] == '&') &&
		array[i-1][0] == '\n')
		error(10,10, "Shrek is love");
	if(i == (*arraySize)-1 && array[i][0] == '\n')
		(*arraySize)--;
//	if(i == strlen(array[i])-1 && isOperand(array[i][strlen(array[i])-1]) 
//			&& (array[i][strlen(array[i])-1]) != '\n')
//		{*arraySize--; error(3,3,"yoloswag");} //last (node) is a \n
	}
}

command_t makeSimpleCommand(command_t result, char* curString){
	/*	char** comString = malloc(sizeof(char*)*DEFAULT_BUFFER_SIZE);
		*comString = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
		strcpy(comString[0], curString);
		//strcpy(*(result->u.word), comString);
		(result->u.word) = comString;
	*/
		char** comString = malloc(sizeof(char*));
		char* temp = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
		//temp = realloc(temp, sizeof(char)*DEFAULT_BUFFER_SIZE);
		//comString = realloc(comString, sizeof(char*));
		
		strcpy(temp, curString);
		*comString = temp;
		
		int k=0;
		int count = 0;
		char* str = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
		char* tmp = malloc(sizeof(char)*2);
		for(k=0; curString[k] != '\0';k++){
			if(curString[k] != '<' && curString[k] != '>'){
			//	tmp[0] = curString[k];
			//	tmp[1] = '\0';
			//strncat(string, tmp, 1);
			str[strlen(str)] = curString[k];
			str[strlen(str)] = '\0';
			}
			else if(curString[k] == '<'){
				count = 1;
				//(result->u.word) = &str;	
				char** tempdoe = malloc(sizeof(char*));
				strcpy(*(tempdoe), str);
				result->u.word = tempdoe;
				str[0] = '\0';
			}
			else if(curString[k] == '>'){
				if(count == 1){
					result->input = str;
					str[0] = '\0';
				}
				count = 2;
				char** tempdoe = malloc(sizeof(char*));
				strcpy(*(tempdoe), str);
				result->u.word = tempdoe;
				str[0] = '\0';
			}
		}

		switch(count){
			case 0:
				char** tempdoe = malloc(sizeof(char*));
				strcpy(*(tempdoe), str);
				result->u.word = tempdoe;
				break;
			case 1:
				result->input = str;
				break;
			case 2:
				result->output = str;
				break;
		}

		return result;
		

		//result->u.word = comString;
//		free(comString);
}

command_t makeCommand(char *curString, int type){
	command_t result = malloc(sizeof(struct command));

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
		makeSimpleCommand(result, curString);
	}

	return result;
}

//for all commands besides simple commands
command_t combineCommand(command_t command1, 
		command_t command2, command_t operator){
	//send in same command twice to indicate subshell operator
	if(command1 == command2)
		operator->u.subshell_command = command1;
	else{
		operator->u.command[0] = command1;
		operator->u.command[1] = command2;
	}
	return operator;
}

int precedence(command_t command){
	if(command->type == SEQUENCE_COMMAND)
//		return 0;
		return 3;
	else if(command->type == PIPE_COMMAND)
		return 1;
//		return 2;
	else if(command->type == SUBSHELL_COMMAND)
		return 2;
//		return 1;
	else //either AND or OR command
//		return 3;
		return 0;


//completely messed up, I think
}

command_stream_t parse(char **stringArray, unsigned int arrSize){
	//initialize operator and command stacks
	struct stack opStack;
	opStack.commands = malloc(sizeof(command_t)*25);
	opStack.size = 0;
	opStack.max_size = 25;
	struct stack comStack;
	comStack.commands = malloc(sizeof(command_t)*25);
	comStack.size = 0;
	comStack.max_size = 25;

	//make node + pointer to current node
	struct node *first = malloc(sizeof(struct node));
	first->next = NULL;
	struct node *curNode;
	curNode = first;

	//go through entire array
	unsigned int k = 0;
	for(k=0 ; k < arrSize ; k++){
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
			command_t topOp = peek(&opStack);
				pop(&opStack);
			while(topOp->type != SUBSHELL_COMMAND){
				command_t command2 = peek(&comStack);
				pop(&comStack);
				command_t command1 = peek(&comStack);
				pop(&comStack);
				command_t newCommand = 
			combineCommand(command1, command2, topOp);
				push(&comStack, newCommand);
				topOp = peek(&opStack);
					pop(&opStack);
			}//TODO: what happens if there's double paren, e.g. ))
			command_t command1 = peek(&comStack);
				pop(&comStack);
			command_t newCommand =
			combineCommand(command1, command1, topOp);
			push(&comStack, newCommand);
			continue;
		}
		
		//we have at least two newlines, make new node
		if(k > 0 && stringArray[k][0] == '\n'  &&
			(stringArray[k-1][0] == '|' || stringArray[k-1][0] == '&'))
			continue;
		if(stringArray[k][0] == '\n' && stringArray[k][1] == '\n'){
			while(peek(&opStack) != NULL){
				command_t operator = peek(&opStack);
				pop(&opStack);
				command_t command2 = peek(&comStack);
				pop(&comStack);
				command_t command1 = peek(&comStack);
				pop(&comStack);
				command_t newCommand = combineCommand(command1, command2, operator);
				push(&comStack, newCommand);
			}
			struct node *newNode = malloc(sizeof(struct node));
			newNode->next = NULL;
			curNode->next = newNode;
			curNode->command = peek(&comStack);
			pop(&comStack);
			curNode = newNode;
			continue;
		}

		command_t curCommand = malloc(sizeof(struct command));
		curCommand = makeCommand(stringArray[k], comType);
		if(curCommand->type == SIMPLE_COMMAND) //not an operator
			push(&comStack, curCommand);
		else if(curCommand->type == SUBSHELL_COMMAND) //encounter open paren
			push(&opStack, curCommand);
		else{
			if(peek(&opStack) == NULL)
				push(&opStack, curCommand);
			else{
				if(precedence(curCommand) > precedence(peek(&opStack)))
					push(&opStack, curCommand);
				else{
					command_t operator = peek(&opStack);
				//	enum command_type comp1 = operator -> type;
				//	enum command_type comp2 = SUBSHELL_COMMAND;
				//	bool con1 = (comp1 == comp2);
				//	bool con2 = (precedence(operator) <= precedence(peek(&opStack))); //debugging
					while((precedence(operator) <= precedence(peek(&opStack)) && (operator->type !=SUBSHELL_COMMAND))){
						operator = peek(&opStack);
						pop(&opStack);
						command_t command2 = peek(&comStack);
						pop(&comStack);
						command_t command1 = peek(&comStack);
						pop(&comStack);
						command_t newCommand = combineCommand(command1, command2, operator);
						push(&comStack, newCommand);
						if(peek(&opStack) == NULL)
							break;
					}
					push(&opStack, curCommand);
				}
			}
		}
	}
	//pop off everything left at the end, should not have any parens/newlines on the stack by now
	//[if there is I fucked up]
	while(peek(&opStack) != NULL){
		command_t operator = peek(&opStack);
		command_t command2 = peek(&comStack);
		pop(&comStack);
		pop(&opStack);
		command_t command1 = peek(&comStack);
		pop(&comStack);
		command_t newCommand = combineCommand(command1, command2, operator);
		push(&comStack, newCommand);
	}

	curNode->command = peek(&comStack);
	pop(&comStack); //dont ask, it works
	struct command_stream *cstream = malloc(sizeof(struct command_stream)); 
	cstream->start = first;
	cstream->iterator = first;

	return cstream;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
	unsigned int x = -1;
	unsigned int * tmpPnt = &x;
	char ** commandArray = lexer(*get_next_byte, 
				get_next_byte_argument,
				tmpPnt);
	unsigned int arraySize = *tmpPnt;
	//returns an array of all the commands
	checkDontShrek(commandArray, &arraySize);
	return parse(commandArray, arraySize);
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  //error (1, 0, "command reading not yet implemented");
  //return 0;
}

command_t
read_command_stream (command_stream_t s)
{
	command_t result;
	if(s->iterator != NULL){
		result = s->iterator->command;
		s->iterator = s->iterator->next;
	}
	else
		return NULL;
	return result;

  /* FIXME: Replace this with your implementation too.  */
  //error (1, 0, "command reading not yet implemented");
  //return 0;
}
