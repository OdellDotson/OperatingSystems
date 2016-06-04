#include <stdio.h>
#include <string>
#include <stack>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

using namespace std;

int badFiles = 0;
int directories = 0;
int regFiles = 0;
int specFiles = 0;
long sizeRegFiles = 0;
int textOnlyRegFiles = 0;
long sizeTextOnly = 0;
int threads;
int a = 1;
std::stack<std::string> files;

sem_t fileStack;

main(int argc, char **argv)
{
	void *procRequest(void *arg);//Create function prototyes
	sem_init(&fileStack, 0,1);//Only one task accesses the stack at a time.

	string lineInput;

	if(argc == 1)
	{
		threads = 1;
	}
	else
		atoi(argv[1]) > 15 ? threads = 15 : threads = atoi(argv[1]);

	printf("Processing directory using %d threads.\n", threads);

	while (getline(cin,lineInput)) {
		//cout << lineInput << endl;
		files.push(lineInput);
	}

	printf("%ld files in the directory to process.\n", files.size());
	pthread_t proc_0, proc_1, proc_2, proc_3, proc_4,proc_5,proc_6,proc_7,proc_8,proc_9,proc_10,proc_11,proc_12,proc_13,proc_14;//Create the threads 
	pthread_t procID[15] = {proc_0, proc_1, proc_2, proc_3, proc_4,proc_5,proc_6,proc_7,proc_8,proc_9,proc_10,proc_11,proc_12,proc_13,proc_14};//Create an array of those threads
	//For each of the inputs, create a new process.
	for(int i = 0; i < threads; i++)
	{	
		pthread_create(&(procID[i]), NULL, procRequest, (void *)a);
	}

	
	//Here we wait for processes to complete.
		/*On a process completion, we should recreate that process with a new job, next in the list. Maximum of 15 processes.*/
	for(int i = 0; i < threads; i++)
	{
		pthread_join(procID[0], NULL); //Whatever the hell goes here. We need to wait on a specific thread ID.
	}
	cout << "Bad files: " << badFiles << endl;
	cout << "Directories: " << directories << endl;
	cout << "Regular files: " << regFiles << endl;
	cout << "Special files: " << specFiles << endl;
	cout << "Size of Regular files: " << sizeRegFiles << endl;
	cout << "Numer of text-only regular files (in bytes): " << textOnlyRegFiles << endl;
	cout << "Size of text-only files (in bytes):" << sizeTextOnly << endl;
}

int testFileType(const string request)//Returns a value based on the file type of the requested file, based on the types we're searching for.
{
	//printf("The file %s is of type number %d\n", request ,fileType);
	return 0;
}

void *procRequest(void *arg)
{
	struct stat filestat;
	int filestatus;
	while(files.size() != 0)
	{
		sem_wait(&fileStack);
		string file = files.top();
		files.pop();
		sem_post(&fileStack);

		filestatus = stat(file.c_str(), &filestat);

		if(filestatus < 0)
			badFiles++;
		else if(S_ISREG(filestat.st_mode))
		{
			regFiles++;
			sizeRegFiles += filestat.st_size;
			cout << "File name: " << file << "  ||| File size:" << filestat.st_size << endl;\

			int infile;
            infile = open(file.c_str(), O_RDONLY);
			if (infile)
			{
			    unsigned char buffer;
			    int cnt = read(infile, &buffer, sizeof(unsigned char));
			    int text = 1;
			    do
			    {
			        if (!isprint(buffer) && !isspace(buffer))
			        {
			            text = 0;
			            break;
			        }
			        cnt = read(infile, &buffer, sizeof(unsigned char));
			    }
			    while (cnt == 1);
			    close(infile);
			    if (text)
			    {
			        sizeTextOnly += filestat.st_size;
			        textOnlyRegFiles++;
			    }
			}

		}
		else if(S_ISDIR(filestat.st_mode))
			directories++;
		else
			specFiles++;


	}
}