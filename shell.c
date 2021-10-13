#include <stdio.h>
#include <string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdlib.h>

#define READ 0
#define WRITE 1
#define BUFSIZE 100


int main (int argc, char *argv[]) {
	char* str[BUFSIZE]; //buffer for holding the commands
	char* args[BUFSIZE]; // holds the tokenized commands from str
	char* pipeargs[BUFSIZE]; //if there's a pipe, this stores
	int background = 0;
	
	printf("Welcome to the shell. Enter a command.\n");
	while(1) //main while
	{
		char* mydir = getcwd(NULL, 0); // for changing and displaying directory
		printf("shell~%s$ ", mydir);
		
		fgets(str, BUFSIZE, stdin);
		if(strcmp(str, "DONE\n") == 0){break;} 
		if(strcmp(str, "\n") == 0 || strcmp(str, "|\n") == 0){continue;} // empty line and single | are breaking my shell - this fixes by ignoring
		
		char *pipes; //create the pipe case - if piping is necessary, we will find out here and split str into groups based on |
		pipes = strtok(str, "|");
		int k = 0;
		while (pipes != NULL)
		{	
			
			pipeargs[k] = pipes;
			pipes = strtok(NULL, "|");
			k++;
		} //close while
		pipeargs[k] = NULL; //man exec says last item should be NULL

		char *pch; //start process to tokenize str
		pch = strtok(pipeargs[0]," \n"); //we use pipeargs[0] so that piping works later
		int i = 0;
		while (pch != NULL) // while there are more tokens to read
		{
			args[i] = pch;
			pch = strtok(NULL, " \n");
			i++;
		} // close while
		args[i] = NULL; //man exec says last item should be NULL

		// three "cases" below - cd, pipe, and standard. I handle them with if/else statements

		if(strcmp(args[0], "cd") == 0)
		{ // need to handle cd case by using chdir
			mydir = args[1];
			chdir(mydir);
			printf("Changing directory to %s\n", mydir);
		} //close if for cd
	
		else if(k > 1)
		{ //handle the pipe case
			int fd[2]; //for pipe read and write
			if (pipe(fd) == -1) { //create a pipe
				perror("pipe failed"); }
			char *pip;
			char *pargs[BUFSIZE];
			pip = strtok(pipeargs[1]," \n"); 
			int m = 0;
			while (pip != NULL) // while there are more tokens to read
			{
				pargs[m] = pip;
				pip = strtok(NULL, " \n");
				m++;
			} // close while
			pargs[m] = NULL; //man exec says last item should be NULL

			pid_t pid2 = fork();
			if(pid2 == (pid_t) -1){perror("fork: error forking"); exit(1);} // error if fork did not work
			if(pid2 == (pid_t) 0 )
			{ //child process
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
				execvp(args[0], args);
				exit(1);
			} //close if for child process

			pid_t pid3 = fork();
			if(pid3 == (pid_t) -1){perror("fork: error forking"); exit(1);} // error if fork did not work
			if(pid3 == (pid_t) 0 )
			{ //child process
				dup2(fd[0], STDIN_FILENO);
				close(fd[0]);
				close(fd[1]);
				execvp(pargs[0], pargs);
				exit(1);
			} //close if for child process

			//parent process if there were pipes
			close(fd[READ]);
			close(fd[WRITE]);
			int cstatus2;
			wait(&cstatus2);
			wait(&cstatus2);
		} //close else if

			
		else 
		{
			pid_t pid = fork();
			if(pid == (pid_t) -1){perror("fork: error forking"); exit(1);} // error if fork did not work

			if(background == 1) //run another fork from a fork to simulate background process - ran out of time before I could fully implement turning the background switch on and off, so this is vestigial. Currently not working, but I am leaving it because it shows the thought process of how I would implement a background process.
			{
				pid_t bgpid = fork();
				if(bgpid == (pid_t) -1){perror("bgfork: error forking"); exit(1);} // error if fork did not work

				if(bgpid == (pid_t) 0)
				{ //child process
					if (execvp(args[0], args) == -1){perror("execevp: background process failed"); exit(1);} //run exec for bg fork
				} //close bgpid if
				exit(1);
			} //close background if

			if(pid == (pid_t) 0 ) 
			{ //child process				
				if (execvp(args[0], args) == -1) 
				{ //report if error
					perror("execvp: error");
				} //close if
				exit(1);
			} //close if
			else 
			{
				if (pid == (pid_t) (-1))
				{ // fork failed
					perror("Fork failed");
					exit(1);
				} //close if
				// parent process
				
				int cstatus;
				pid_t c = wait(&cstatus);
				//printf("Parent: Child %ld exited with status = %d\n", (long) c, cstatus);
			}
		} //close else 
		} //close main while loop
	return 0;}
