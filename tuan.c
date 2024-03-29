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

void executingAnd(command_t){
  pid_t firstPid;
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
        if(secondPid < 0)
          error(1, errno, "fork was unsuccessful");
        else if(secondPid == 0){ //inside child process
          exectute_switch(c->u.command[1]);
          _exit(c->u.command[1]->status); 
        }
        else{
          //inside parent process
          waitpid(secondPid, &eStatus, 0);
          c->status = c->u.command[1]->status;
        }
    }
  }

}

void executingOr(command_t c){
  pid_t firstPid;
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
          exectute_switch(c->u.command[1]);
          _exit(c->u.command[1]->status); 
        }
        else{
          //inside parent process
          waitpid(secondPid, &eStatus, 0);
          c->status = c->u.command[1]->status;
        }
    }
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
    execute_switch(c->u.subshell_command);
    _exit(c->u.subshell_command->status);
  }
  else{
    waitpid(pid, &eStatus, 0);
    c->status = c->u.subshell_command->status;
  }
}

void executingSimple(command_t c){
  int count = -1;
  char* command;
  char** comargs;
  int k = 0;
  char* curString;
  pid_t pid;
  int eStatus;

  pid = fork();
  if(pid == 0){
    for(k=0; c->u.word[k] != '\0'; k++){
      if(c->u.word[k] != ' ')
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
    }
  }
  else if(pid > 0){
    waitpid(pid, &eStatus, 0);
    c->status = eStatus;
  }
  else{
    error(1, errno, "fork was unsuccessful");
  }

  execvp(command, comargs);
}

void
execute_command (command_t c, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  error (1, 0, "command execution not yet implemented");
}
