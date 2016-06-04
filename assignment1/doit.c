/* Odell Dotson Assigment 1 CS3013: Operating Systems

This program allows the user to input commands and see information about those commands.

If the command is given with the initialization of the program, it will run that command, print information about it, and then this program
will terminate.

If no command is given, this program will act as a shell, receiving commands until it receives the special command "exit", or if it receives 
an empty line input.

Users can also use the special command "cd" to change directories.

Commands with the & argument given after them, the program will run in the background, and allow other inputs from the user.
(Sadly commands run in the background gives iffy statistics.)
(Sadly background processes also must be killed individually.)

*/
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

extern char **environ;		/* environment info */

main(int argc, char **argv)
{
    char *argvNew[32];
    int pid;
    struct timeval start, end;//Time value structures for calculating the wallclock time
	struct rusage usageStart, usageEnd;//Usage statistic structures
	gettimeofday(&start, NULL);//Take an initial wall clock reading
	getrusage(RUSAGE_CHILDREN, &usageStart);//Take an initial statistics reading on children
    
    if ((pid = fork()) < 0) 
    {//If we can't make a new process, error away.
        fprintf(stderr, "Fork error\n");
        exit(1);
    }

    else if (pid == 0)         /* child process */
    {
		if(argc == 1)//Only one argument, shell mode.
		{
			int printStats = 0;//Checks if this is the first command.

			printf("%s", ">");
			while(1)//Persist shell after commands executed.
			{
				char line[128]={0};//Input buffer
				int j;//Used for for looping
				getrusage(RUSAGE_SELF, &usageEnd);//update the end usage statistics
				gettimeofday(&end, NULL);//Update end wallclock time

				int status;//Status from waitpid
                pid_t pidResult = waitpid(pid, &status, WNOHANG);//Kill any things that might still be hanging

				if (printStats){//Makes sure we don't try to print output info before any tasks have been run
					if (pidResult <= 0) //Child running
	                {
	                	wait(0);//Wait for the child to finish, then print it's stats.
	                	printf("\nClock time elapsed:%ld\n",    (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000));	
						printf("CPU Time (User):%ld\n", usageEnd.ru_utime.tv_sec * 1000 + usageEnd.ru_utime.tv_usec / 1000);
						printf("CPU Time (System):%ld\n", usageEnd.ru_stime.tv_sec * 1000 + usageEnd.ru_stime.tv_usec / 1000);
	    				printf("Voluntary context switches: %ld\n", usageEnd.ru_nvcsw - usageStart.ru_nvcsw);
	    				printf("Involuntary context switches: %ld\n", usageEnd.ru_nivcsw - usageStart.ru_nivcsw);
	    				printf("Number of minor page faults: %ld\n", usageEnd.ru_minflt - usageStart.ru_minflt);
	    				printf("Number of major page faults: %ld\n\n", usageEnd.ru_majflt) - usageStart.ru_majflt;
						printf("%s", ">");
	                }
	                else //Error from child.
	                {
	                	printf("%s\n", "pidResult > 0, error forking.");
						printf("%s", ">");
	                }
				}

				if (fgets(line, sizeof(line), stdin)) //Check if we've received an input, and put it to "line"
				{//If we've received input, execute it.
    				gettimeofday(&start, NULL);//Update time start for a subprocess
    				getrusage(RUSAGE_SELF, &usageStart);//Update resource data
    				printStats = 1;//Actually print statistics from now on
					char *arg;//Used for individual arguments put into the line
					int argcNew = 0;//Counter for new args
					const char s[2] = " ";//A character we want to split the input into tokens with
					const char br[1] = "\n";//Another character for us to split input into tokens on
					arg = strtok(line, s);//Break the first argument off
					argvNew[0] = arg;//Put it into the first argument position
				   	for(j=0;arg != NULL;j++)//Populate argvNew with the arguments
				   	{
						arg = strtok(NULL, s);//Continue the splitting process
						argcNew++;//increment the number of given arguments
						argvNew[j+1] = arg;//Push the new argument to the list of arguments,+1 because we do 0 up there
		  		 	}

		  		 	argvNew[argcNew-1] = strtok(argvNew[argcNew-1], br);//Remove the line ending thing from the last command
					argvNew[argcNew] = NULL;//Make a null to terminate the list after the last command.

					/*Test foregroundiness of processes so they run where they're supposed to.*/
					int foregroundiness = 1;
					for(j=0;argvNew[j] != NULL; j++)//Search for & through the input
					{
						if(strncmp(argvNew[j], "&", 1) == 0)
						{
							foregroundiness = 0;//If we find it, change the foregroundiness for later
							argvNew[j] = NULL;//Replace the & with null for actual running.
						}
					}

					if(strncmp("exit",argvNew[0],4) == 0)// If we get the exit command, exit the shell.
					{
						
						kill (getppid(), 9);//Kill the parent
						exit(1);//exit
					}

					else if(strncmp("cd", argvNew[0],2) ==0)//Change directories
					{
						chdir(argvNew[1]);//Change directories to the given argument location
					}



				    else if ((pid = fork()) < 0) {//Create a new fork for the new commands
				        fprintf(stderr, "Fork error\n");
				        exit(1);
				    }
				    else if (pid == 0) {//Do the new things
				    	wait(0);
				        if (execvp(argvNew[0], argvNew, environ) < 0) 
				        {
				            fprintf(stderr, "Are you sure you typed that command right?\n");
				            exit(1);
				        }
				    }
				    
				    else 
				    {// parent 
						if(foregroundiness)
						{
							wait(0);  // wait for the child to finish if we're in the foreground
							
               				pid_t pidResult = waitpid(pid, &status, WNOHANG);
						}
						else//If we're sending a program to the background
						{
							argvNew[0] = "./doit";//Make new arguments for a new fork with the new process
							argvNew[1] = NULL;
							if ((pid = fork()) < 0) 
							{//Create a new fork for the new commands
						        fprintf(stderr, "Fork error\n");
						        exit(1);
						    }
						    if(strncmp("exit",argvNew[0],4) == 0)// If we get the exit command, exit the shell.
							{
								exit(1);          
							}

							else if(strncmp("cd", argvNew[0],2) ==0)//Change directories
							{
								chdir(argvNew[1]);
							}

						    else if (pid == 0) {//Do the new things in the new process
						    	wait(0);
						        if (execvp(argvNew[0], argvNew, environ) < 0) 
						        {
						            fprintf(stderr, "Are you sure you typed that command right?\n");
						            exit(1);
						        }
						    }
               				pid_t pidResult = waitpid(pid, &status, WNOHANG);//Break out of any finished children
						    getrusage(RUSAGE_CHILDREN, &usageEnd);//Get information about the child's usage, once we're sure it's done
						}
					}
				}
			}
		}

		else{//We received commands, we're not a shell. We're still a child though. Prepare the commands.
			int i;//Used for looping
			int pidNew;//Stores the new PID value
			
			for(i=0;i<argc;i++)
			{
				argvNew[i] = argv[i+1];
			}


			getrusage(RUSAGE_CHILDREN, &usageStart);//Update usage data at start
			gettimeofday(&start, NULL);//Update wallblock time at start
			if (execvp(argvNew[0], argvNew, environ) < 0) 
				{
	        	    fprintf(stderr, "Execvp error\n");
	        	    exit(1);
			    }	
		    wait(0);//Wait for child to finish
			getrusage(RUSAGE_CHILDREN, &usageEnd);//Update usage data at end
			gettimeofday(&end, NULL);//Update usage data at end
			printf("\n\nClock time elapsed:%ld\n",    (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000));
			printf("System time elapsed: %ld\n", usageEnd.ru_stime.tv_usec);
			printf("Voluntary context switches: %ld\n", usageEnd.ru_nvcsw - usageStart.ru_nvcsw);
			printf("Involuntary context switches: %ld\n", usageEnd.ru_nivcsw - usageStart.ru_nivcsw);
			printf("Number of minor page faults: %ld\n", usageEnd.ru_minflt - usageStart.ru_minflt);
			printf("Number of major page faults: %ld\n\n", usageEnd.ru_majflt) - usageStart.ru_majflt;
		}
    }
    else 
    {
        /* parent */
		wait(0);		/* wait for the child to finish */
		getrusage(RUSAGE_CHILDREN, &usageEnd);//Update ending stats
		gettimeofday(&end, NULL);//Update ending time
		


		if(pid != 0)//If we're not the child, print information about the processes
		{
		printf("\n\nClock time elapsed:%ld\n",    (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000));
		printf("CPU Time (User):%ld\n", usageEnd.ru_utime.tv_sec * 1000 + usageEnd.ru_utime.tv_usec / 1000);
		printf("CPU Time (System):%ld\n", usageEnd.ru_stime.tv_sec * 1000 + usageEnd.ru_stime.tv_usec / 1000);
		printf("Voluntary context switches: %ld\n", usageEnd.ru_nvcsw - usageStart.ru_nvcsw);
		printf("Involuntary context switches: %ld\n", usageEnd.ru_nivcsw - usageStart.ru_nivcsw);
		printf("Number of minor page faults: %ld\n", usageEnd.ru_minflt - usageStart.ru_minflt);
		printf("Number of major page faults: %ld\n\n", usageEnd.ru_majflt) - usageStart.ru_majflt;	
		}
    }
}
