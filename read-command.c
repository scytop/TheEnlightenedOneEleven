// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <error.h>


#define DEFAULT_BUFFER_SIZE 500
/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */



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

char * currentString = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
char ** stringArray = malloc(sizeof(char*) * DEFAULT_BUFFER_SIZE);
int currentPos = 0;
int maxArrayElem = 0;

while( c = get_next_byte(get_next_byte_argument) && c != EOF)
	{
		//assume that only valid inputs are allowed
		if (c == '(' || c == ')' || c == ';')
		{
		//These are singular operands, always, so this should push
		//and create a new cstring
		strcat(currentString, c);
		strcat(currentString, '\0');
		stringArray[maxArrayElem] = currentString;
		currentString = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
		currentPos = 0;
		maxArrayElem++;
		prev_c = '\0' //kind of hacky, but ensures the next iteration will
									//be an append instead of a string-push
		}
		else if(prev_c == '\0' || 
					(isOperand(c) && isOperand(prev_c))
			)
			{ //if the current string needs to be appended
				//FIXME: implement what happens if they go over 500 chars
				strcat(currentString, c);
				currentPos++;
				prev_c = c;

			}
		else if(isOperand(c) != isOperand(prev_c))
		{ //if we need to push the current string to the stringArray
			strcat(currentString, '\0');
			stringArray[maxArrayElem] = currentString; //pushes c string
			currentString = malloc(sizeof(char)*DEFAULT_BUFFER_SIZE);
				//creates a new c string 
			currentPos = 0;
			strcat(currentString, c);
			maxArrayElem++;
			prev_c = c;
		}
	}
*arraySize = maxArrayElem+1;
return stringArray;

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
	

  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
