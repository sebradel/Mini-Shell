//Sebastian Radel
//CS444 Proj3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

//accept up to 16 command-line arguments
#define MAXARG 16

//allow up to 64 environment variables
#define MAXENV 64


//accept up to 1024 bytes in one command
#define MAXLINE 1024

//Copy of envp array
char *envpCpy[64];

//Array of command history
char *cmdHistory[500];

//Points to a copy of most recent command
char *cmdLinePtr;

//Total count of commands (stops incrementing once value reaches 500)
int cmdCount = 0;

//Total count of environment variables
int envCount = 0;


static char **parseCmd(char cmdLine[]) {
  char **cmdArg, *ptr;
  int i;

  //(MAXARG + 1) because the list must be terminated by a NULL ptr
  cmdArg = (char **) malloc(sizeof(char *) * (MAXARG + 1));
  if (cmdArg == NULL) {
    perror("parseCmd: cmdArg is NULL");
    exit(1);
  }
  for (i = 0; i <= MAXARG; i++) //note the equality
    cmdArg[i] = NULL;
  i = 0;
  ptr = strsep(&cmdLine, " ");
  while (ptr != NULL) {
    // (strlen(ptr) + 1)
    cmdArg[i] = (char *) malloc(sizeof(char) * (strlen(ptr) + 1));
    if (cmdArg[i] == NULL) {
      perror("parseCmd: cmdArg[i] is NULL");
      exit(1);
    }
    strcpy(cmdArg[ i++ ], ptr);
    if (i == MAXARG)
      break;
    ptr = strsep(&cmdLine, " ");
  }
  return(cmdArg);
}

int main(int argc, char *argv[], char *envp[]) {
  
  char cmdLine[MAXLINE], **cmdArg;
  int status, i, debug;
  pid_t pid;

  for(int i=0; i<500; i++) {
    cmdHistory[i] = malloc(1024);
  }

  while(envp[envCount]!=NULL) {
    char *tempPtr = malloc(1024);
    strcpy(tempPtr, envp[envCount]);    
    envpCpy[envCount] = tempPtr;
    envCount++;
  }

  debug = 0;
  i = 1;
  while (i < argc) {
    if (! strcmp(argv[i], "-d") )
      debug = 1;
    i++;
  }
  while (( 1 )) {
    printf("bsh> ");                      //prompt
    fgets(cmdLine, MAXLINE, stdin);       //get a line from keyboard
    cmdLine[strlen(cmdLine) - 1] = '\0';  //strip '\n'
    cmdLinePtr = malloc(1024);
    strcpy(cmdLinePtr, cmdLine);
    cmdArg = parseCmd(cmdLine);
    if (debug) {
      i = 0;
      while (cmdArg[i] != NULL) {
	printf("\t%d (%s)\n", i, cmdArg[i]);
	i++;
      }
    }

    //built-in command exit
    if (strcmp(cmdArg[0], "exit") == 0) {
      if (debug)
	printf("exiting\n");
      break;
    }
    //built-in command env
    else if (strcmp(cmdArg[0], "env") == 0) {
      for(int i=0; i<envCount; i++) {
        printf("%s\n", envpCpy[i]);
      }
    }
    //built-in command setenv
    else if (strcmp(cmdArg[0], "setenv") == 0) {
      int found = 0, foundIndex = 0;
      char *newEnv;
      for(int i=0; i<envCount; i++) {
	//Separate env variable string
        char tempStr[1024], *justVar, *justVal;
	strcpy(tempStr, envpCpy[i]);
	justVal = tempStr;
        justVar = strsep(&justVal, "=");
	if(strcmp(justVar, cmdArg[1])==0) {
	  found = 1;
	  foundIndex=i;
	  break;
	}
      }
      
      //Dynamically allocate space for a new string
      newEnv = malloc(sizeof(char) * (strlen(cmdArg[1]) + 
			strlen(cmdArg[2]) + 1));
      strcpy(newEnv, cmdArg[1]);
      strcat(newEnv, "=");
      strcat(newEnv, cmdArg[2]);
      if(found) {
	free(envpCpy[foundIndex]);
      	envpCpy[foundIndex] = newEnv;
      } else {
      	envpCpy[envCount] = newEnv;
	envCount++;
      }
      
    }
    //built-in command unsetenv
    else if (strcmp(cmdArg[0], "unsetenv") == 0) {
      int found = 0, foundIndex = 0;
      for(int i=0; i<envCount; i++) {
      	char tempStr[1024], *justVar, *justVal;
	strcpy(tempStr, envpCpy[i]);
	justVal = tempStr;
	justVar = strsep(&justVal, "=");
	if(strcmp(justVar, cmdArg[1])==0) {
  	  found = 1;
	  foundIndex = i;
	  break;
	}
      }
      if(found) {
      	for(int i=foundIndex; i<envCount; i++) {
	  free(envpCpy[i]);
	  if(i<envCount-1) {
	    char *tempPtr;
	    tempPtr = malloc(1024);
	    strcpy(tempPtr, envpCpy[i+1]);
	    envpCpy[i] = tempPtr;
	  }
	}
	envCount--;
      }
    }
    //built-in command cd
    else if (strcmp(cmdArg[0], "cd") == 0) {
      int found = 0, pwdIndex = 0;
      char *pwd;

      //If command is 'cd' with no additional arguments, change to home directory
      if((cmdArg[1] == NULL) || ((strcmp(cmdArg[1], "") == 0 ))) {
        for(int i=0; i<envCount; i++) {
	  char tempStr[1024], *justVar, *justVal;
	  strcpy(tempStr, envpCpy[i]);
	  justVal = tempStr;
	  justVar = strsep(&justVal, "=");
	  if(strcmp(justVar, "HOME")==0) {
	    found = 1;
	    chdir(justVal);
            pwd = malloc(1024);
	    strcpy(pwd, justVal);
	  }else if (strcmp(justVar, "PWD")==0){
	    pwdIndex = i;
	  }
	}
	if(found) {
	  char *tempPtr;
	  tempPtr = malloc(1024);
	  strcpy(tempPtr, "PWD");
	  strcat(tempPtr, "=");
	  strcat(tempPtr, pwd);
	  free(envpCpy[pwdIndex]);
	  envpCpy[pwdIndex] = tempPtr;
	}
      //If command has additional arguments, change to specified directory	
      }else{
        for(int i=0; i<envCount; i++) {	
	  char tempStr[1024], *justVar, *justVal;
	  strcpy(tempStr, envpCpy[i]);
	  justVal = tempStr;
	  justVar = strsep(&justVal, "=");
	  if(strcmp(justVar, "PWD")==0) {
	    found = 1;
	    pwdIndex = i;
	  }
	}
	chdir(cmdArg[1]);
	char *tempPtr;
	char cwd[1024];
	char *cwdPtr = malloc(1024);
	getcwd(cwd, sizeof(cwd));
	strcpy(cwdPtr, cwd);
	tempPtr = malloc(1024);
	strcpy(tempPtr, "PWD");
	strcat(tempPtr, "=");
	strcat(tempPtr, cwdPtr);
	if(found) {
	  free(envpCpy[pwdIndex]);
	  envpCpy[pwdIndex] = tempPtr;
	}else{
	  envpCpy[envCount] = tempPtr;
	  envCount++;
	}
      }  
    }
    //built-in command history
    else if (strcmp(cmdArg[0], "history") == 0) {
      for(int i=0; i<cmdCount; i++) {
        printf("%s\n", cmdHistory[i]);
      }
    }
    
    char *tempPtr = malloc(1024);
    //Update the command history array
    //If array is full, shift it down first
    if(cmdCount<500) {
      cmdHistory[cmdCount] = cmdLinePtr;
      cmdCount++;
    } else {
      for(int i=0; i<499; i++) {
	free(cmdHistory[i]);
        cmdHistory[i] = cmdHistory[i+1];
      }
      cmdHistory[499] = cmdLinePtr;
    }

    //clean up before running the next command
    i = 0;
    while (cmdArg[i] != NULL)
      free( cmdArg[i++] );
    free(cmdArg);
  }

  return 0;
}
