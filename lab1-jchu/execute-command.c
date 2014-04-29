// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#define GRAPH_SIZE 25

struct graphNode{
command_t cmd;
pid_t pid;
struct graphNode **before;
int beforeSize;
};

struct string_q{
char* str;
struct string_q* next;
};

struct listNode{
graphNode* gnode;
string_q* readlist; //don't forget to malloc these!
string_q* writelist;
struct listNode* next;
};

struct dependencyGraph{
listNode** no_dependencies;
listNode** dependencies;
int ndSize;
int dSize;
};
/*
bool hasSimilar(string_q* l1, string_q* l2)
{
//returns true if there exists an element that occurs
//in both l1 and l2
string_q* t1 = l1;
string_q* t2 = l2;

if(t1 == NULL)
	return false;
else if (t2 == NULL)
{
return hasSimilar(t1->next, l2);
}
if(strcmp(t1->str, t2->str) == 0)
	return true;
else
	return(t1, t2->next); 
}
*/
bool hasSimilar(string_q* l1, string_q* l2)
{
string_q* start = l2;
while(l1 != NULL)
{
	while(l2 != NULL)
	{
	if(strcmp(l1->str, l2->str) == 0)
		return true;
	else
		l2 = l2->next;
	}
l2 = start;
l1 = l1->next;
}
return false;
}

void linsert(string_q* root, char* elem)
{
string_q* temp = root;

if (root == NULL)
{//if it's the first element to be put in
root = malloc(sizeof(string_q));
string_q* newGuy = malloc(sizeof(string_q));
newGuy-> next = NULL;
root-> next = newGuy;
char* copyTo = malloc(sizeof(char)*300);
strcpy(copyTo, elem);
newGuy->str = copyTo;
return;
}

while(temp != NULL)
{
if(temp->next == NULL)
{
	string_q* newGuy = malloc(sizeof(string_q));
	newGuy->next = NULL;
	temp-> next = newGuy;
	char* copyTo = malloc(sizeof(char)*300);
	strcpy(copyTo, elem);	
	newGuy->str = copyTo;
	break;
}
else
{
temp = temp->next;
}
}
}//not efficient rn, can come back later to "insert sort"

dependencyGraph* buildItBro(listNode* start)
{
dependencyGraph* result = malloc(sizeof(dependencyGraph));
result->ndSize = 0;
result->dSize  = 0;
result->no_dependencies = malloc(sizeof(listNode*)*150);
result->dependencies = malloc(sizeof(listNode*)*150);
listNode* current = start;
listNode* checkNode = malloc(sizeof(listNode));

	if(current != NULL) checkNode = current->next;
	while(current->next != NULL)
	{
	while(checkNode->next !=NULL)
	{
	if(hasSimilar(current->writelist, checkNode->readlist)||
		hasSimilar(current->readlist, checkNode->writelist)||
		hasSimilar(current->writelist, checkNode->writelist))
	{
	checkNode->gnode->before[(checkNode->gnode->beforeSize)] = current->gnode;
	(checkNode->gnode->beforeSize)++;
	}
	checkNode = checkNode->next;
	}
	current = current->next;
	}
	//ok all these should have their deps
	
	while(start != NULL)
	{
	if(start->gnode->beforeSize == 0)
	{
		(result->no_dependencies)[result -> ndSize] = start;
		(result->ndSize)++;
	}
	else
	{
		(result->dependencies)[result -> dSize] = start;
		(result->dSize)++;
	}
	start = start->next;
	}
return result;
}


void proCom(command_t c, listNode* l){
if(c->type == SIMPLE_COMMAND){
	if(c->input != NULL) linsert(l->readlist, c->input);
	int i = 0;
	while(c->u.word[i] != NULL)//FIXME: THIS IS WHERE WE GET MESSED UP
	{
	linsert(l->readlist, c->u.word[i]);
	i++;
	}
	if(c->output != NULL) linsert(l->writelist, c->output);}
else if(c->type == SUBSHELL_COMMAND)
{
	if(c->input != NULL) linsert(l->readlist, c->input);
	if(c->input != NULL) linsert(l->writelist, c->output);
	proCom(c->u.subshell_command, l);
}
else
{
	proCom(c->u.command[0], l);
	proCom(c->u.command[1], l);
}
}

//	dependencyGraph* result = malloc(sizeof(dependencyGraph));
//	result->ndSize = 0;
//	result->dSize  = 0;
dependencyGraph* create_graph(command_stream_t cs){
	listNode* current = malloc(sizeof(listNode));
	listNode* start = current;
	current->next = NULL;

	command_t command;
	//build the first linked list of all listNodes
	while((command = read_command_stream(cs)))
	{
	listNode* l = malloc(sizeof(listNode));
	proCom(command, l);
	graphNode* g = malloc(sizeof(graphNode));
	g->cmd = command;
	g->beforeSize = 0;
	g->before = malloc(250*sizeof(graphNode*));
	l->gnode = g;
	//builds a single node	

	current-> next = l;
	l->next= NULL;
	current = l;
	//add it to the list
	}
	start = start->next;
	//start will be null if there are no thinggies
	//start has a linked list of read/write listnodes
	//build it forward, not backwards
	return buildItBro(start);
}

void execNd(dependencyGraph* d)
{
int i = 0;
int size = d->ndSize;
for(i = 0; i < size; i++)
{
	pid_t pid = fork();
	if(pid == 0)
	{
	execute_command((d->no_dependencies)[i]->gnode->cmd, true);
	_exit((d->no_dependencies)[i]->gnode->cmd->status);
	}
	if(pid >0)
	{
	(d->no_dependencies)[i] ->gnode->pid = pid;
	}
}
}


void execD(dependencyGraph* d)
{
int temp;
int i;
int size = d->dSize;
for(i = 0; i<size; i++)
{
	int j = 0;
	for(j = 0; d->dependencies[i]->gnode->before[j] != NULL; j++)
	{
	if(d->dependencies[i]->gnode->before[j]->pid == -1)
		j = -1;
	else
		continue;
	}
	for(j = 0; d->dependencies[i]->gnode->before[j] != NULL; j++)
	{
		waitpid(d->dependencies[i]->gnode->before[j]->pid, &temp, 0);
	}
	pid_t pid = fork();
	if(pid < 0)
		error(1, errno, "It Forked up");
	else if(pid == 0)
	{
	execute_command(d->dependencies[i]->gnode->cmd, true);
	_exit(d->dependencies[i]->gnode->cmd->status);
	}
	else
		d->dependencies[i]->gnode->pid = pid;
}
}

void execGraph(dependencyGraph* d)
{
execNd(d);
execD(d);

int i=0;
int j = 0;
int temp;
for(i = 0; i < d->ndSize; i++)
{
	waitpid(d->no_dependencies[i]->gnode->pid, &temp, 0);
}
for(j = 0; j < d->dSize; j++)
	waitpid(d->dependencies[j]->gnode->pid, &temp, 0);
}
int
command_status (command_t c)
{
  return c->status;
}	

void executingSimple(command_t c);
void executingSubshell(command_t c);
void executingAnd(command_t c);
void executingOr(command_t c);
void executingSequence(command_t c);
void executingPipe(command_t c);

void execute_switch(command_t c)
{
  switch(c->type)
  {
    case SIMPLE_COMMAND:
      executingSimple(c);
      break;
    case SUBSHELL_COMMAND:
      executingSubshell(c);
      break;
     case AND_COMMAND:
      executingAnd(c);
      break;
    case OR_COMMAND:
      executingOr(c);
      break;
    case SEQUENCE_COMMAND:
      executingSequence(c);
      break;
    case PIPE_COMMAND:
      executingPipe(c);
      break;
    default:
      error(1, 0, "Not a valid command");
  }
}

void executingPipe(command_t c)
{
  pid_t returnedPid;
  pid_t firstPid;
  pid_t secondPid;
  int buffer[2];
  int eStatus;

  if ( pipe(buffer) < 0 )
  {
    error (1, errno, "pipe was not created");
  }

  firstPid = fork();
  if (firstPid < 0)
  {
    error(1, errno, "fork was unsuccessful");
  }
  else if (firstPid == 0) //child executes command on the right of the pipe
  {
    close(buffer[1]); //close unused write end
           //redirect standard input to the read end of the pipe
           //so that input of the command (on the right of the pipe)
           //comes from the pipe
    if ( dup2(buffer[0], 0) < 0 )
    {
      error(1, errno, "error with dup2");
    }
    execute_switch(c->u.command[1]);
    _exit(c->u.command[1]->status);
  }
  else 
  {
  // Parent process
    secondPid = fork(); //fork another child process
                       //have that child process executes command on the left of the pipe
    if (secondPid < 0)
    {
      error(1, 0, "fork was unsuccessful");
    }
    else if (secondPid == 0)
    {
      close(buffer[0]); //close unused read end
      if(dup2(buffer[1], 1) < 0) //redirect standard output to write end of the pipe
      {
        error (1, errno, "error with dup2");
      }
      execute_switch(c->u.command[0]);
      _exit(c->u.command[0]->status);
    }
    else
    {
      // Finishing processes
      returnedPid = waitpid(-1, &eStatus, 0); //this is equivalent to wait(&eStatus);
                      //we now have 2 children. This waitpid will suspend the execution of
                      //the calling process until one of its children terminates
                      //(the other may not terminate yet)
      //Close pipe
      close(buffer[0]);
      close(buffer[1]);
      if (secondPid == returnedPid )
      {
        //wait for the remaining child process to terminate
        waitpid(firstPid, &eStatus, 0); 
        c->status = WEXITSTATUS(eStatus);
        return;
      }
      if (firstPid == returnedPid)
      {
        //wait for the remaining child process to terminate
        waitpid(secondPid, &eStatus, 0);
        c->status = WEXITSTATUS(eStatus);
        return;
      }
    }
  }
}

void executingAnd(command_t c){
  /*pid_t firstPid;
  pid_t secondPid;
  int eStatus;

  firstPid = fork();
  if (firstPid < 0)
  {
    error(1, errno, "fork was unsuccessful");
  }
  else if (firstPid == 0) 
  {  
    //inside the child process

    //do the first command
    execute_switch(c->u.command[0]);
    _exit(c->u.command[0]->status);
  }
  else 
  {
    //in the parent process
    waitpid(firstPid, &eStatus, 0);
    if(c->u.command[0]->status != 0) //check to see if first command failed
      c->status = -1; 
    else{ //check second command if first command succeeded
      secondPid = fork();
	waitpid(secondPid, &eStatus, 0);
        if(secondPid < 0)
          error(1, errno, "fork was unsuccessful");
        else if(secondPid == 0){ //inside child process
          execute_switch(c->u.command[1]);
          _exit(c->u.command[1]->status); 
        }
        else{
          //inside parent process
          waitpid(secondPid, &eStatus, 0);
          c->status = c->u.command[1]->status;
        }
    }
  }*/
    execute_switch(c->u.command[0]);
    if(c->u.command[0]->status != 0) //check to see if first command failed
      c->status = -1;
    else{
      execute_switch(c->u.command[1]);
      c->status = c->u.command[1]->status;
    }

}

void executingOr(command_t c){
  /*pid_t firstPid;
  pid_t secondPid;
  int eStatus;

  firstPid = fork();
  if (firstPid < 0)
  {
    error(1, errno, "fork was unsuccessful");
  }
  else if (firstPid == 0) 
  {  
    //inside the child process

    //do the first command
    execute_switch(c->u.command[0]);
    _exit(c->u.command[0]->status);
  }
  else 
  {
    //in the parent process
    waitpid(firstPid, &eStatus, 0);
    if(c->u.command[0]->status == 0) //check to see if first command succeeded
      c->status = 0; 
    else{ //check second command if first command failed
      secondPid = fork();
        if(secondPid < 0)
          error(1, errno, "fork was unsuccessful");
        else if(secondPid == 0){ //inside child process
          execute_switch(c->u.command[1]);
          _exit(c->u.command[1]->status); 
        }
        else{
          //inside parent process
          waitpid(secondPid, &eStatus, 0);
          c->status = c->u.command[1]->status;
        }
    }
  }*/
execute_switch(c->u.command[0]);
    if(c->u.command[0]->status == 0) //check to see if first command succeeded
      c->status = 0;
    else{
      execute_switch(c->u.command[1]);
      c->status = c->u.command[1]->status;
    }

}

void executingSequence(command_t c){
  pid_t firstPid;
  pid_t secondPid;
  int eStatus;

  firstPid = fork();
  if(firstPid < 0)
    error(1, errno, "fork was unsuccessful");
  else if(firstPid == 0){
    //inside child process
    execute_switch(c->u.command[0]);
    _exit(c->u.command[0]->status);
  }
  else{
    //inside parent process
    waitpid(firstPid, &eStatus, 0);
    secondPid = fork();
    if(secondPid < 0)
      error(1, errno, "fork was unsuccessful");
    else if(secondPid == 0){
      execute_switch(c->u.command[1]);
      _exit(c->u.command[1]->status);
    }
    else{
      waitpid(secondPid, &eStatus, 0);
      c->status = c->u.command[1]->status; 
      //status of sequence command set to second command
    }
  }
}

void executingSubshell(command_t c){
  pid_t pid;
  int eStatus;

  pid = fork();
  if(pid < 0)
    error(1, errno, "fork was unsuccessful");
  else if(pid == 0){
    //execute_switch(c->u.subshell_command);

    if (c->input != NULL){
      int in = open(c->input, O_RDWR);
      dup2(in, 0);
      close(in);
    }
    if (c->output != NULL){
      int out = open(c->output, O_CREAT | O_WRONLY | O_TRUNC,
          S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      dup2(out, 1);
      close(out);
    }

    execute_switch(c->u.subshell_command);
    _exit(c->u.subshell_command->status);
  }
  else{
    waitpid(pid, &eStatus, 0);
    c->status = c->u.subshell_command->status;
  }
}

void executingSimple(command_t c){


  
 // int count = -1;
  //char* command = malloc(sizeof(char)*50);
  //char** comargs = malloc(sizeof(char*)*10);
  //int k = 0;
 // char* curString;
  pid_t pid;
  int eStatus;

  pid = fork();
  if(pid == 0){
    /*for(k=0; c->u.word[k] != '\0'; k++){
      if(c->u.word[0][k] != ' ')
        strcat(curString, c->u.word[k]);
      else{
        if(count == -1){
          count++;
          command = curString;
          curString = "\0";
        }
        else{
          comargs[count] = curString;
          curString = "\0";
          count++;
        }
      }
    }*/
    if (c->input != NULL){
      int in = open(c->input, O_RDWR);
      dup2(in, 0);
      close(in);
    }
    if (c->output != NULL){
      int out = open(c->output, O_CREAT | O_WRONLY | O_TRUNC,
          S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      dup2(out, 1);
      close(out);
    }
  execvp(c->u.word[0], c->u.word);
  c->status = -1;
 }
  else if(pid > 0){
    waitpid(pid, &eStatus, 0);
    c->status = eStatus;
  }
  else{
    error(1, errno, "fork was unsuccessful");
  }

}


void
execute_command (command_t c, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  //error (1, 0, "command execution not yet implemented");
     time_travel = false;
     execute_switch(c);
}
