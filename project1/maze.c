#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAXRATS 5
#define MAXROOMS 8

sem_t room_0, room_1, room_2, room_3, room_4, room_5, room_6, room_7;//Create semaphores for the rooms
sem_t* roomSems[MAXROOMS] = {&room_0, &room_1, &room_2, &room_3, &room_4, &room_5, &room_6, &room_7};//Create an array to hold the semaphores for the rooms so that we can refer to them generally

int runningRats = 0;//Keeps track of how many rats we have running in the maze
int roomData[MAXROOMS][2];//For each room, store the capacity and time it takes to cross
int roomCount = 0;//Keep a record of the total number of rooms input

struct vbentry {//Struct to hold the specific log of a rat in a room
int iRat; /* rat identifier */
int tEntry; /* time of entry into room */
int tDep; /* time of departure from room */
};

struct vbentry RoomVB[MAXROOMS][MAXRATS]; /* array of room visitors books */
int roomAccessed[MAXROOMS][MAXRATS] = {0};//Keeps track of which rats have been in which rooms. Similar to RoomVB but simply 0 or 1 rather than struct

int InOrder = 1;//Are we in InOrder mode?
int Nonblocking = 0;//Are we in Nonblocking mode?

int initTime;//Initial time
int totalTime = 0;//Total time spent by a rat
int idealTime = 0;//Ideal time spent by all rats

int value;//The value of the semaphore being tested for nonblocking mode.

main(int argc, char **argv)
{
	void *ratrun(void *);//Create function prototyes
	int TryToEnterRoom(int iRat, int iRoom);
	void EnterRoom(int iRat, int iRoom);
	void LeaveRoom(int iRat, int iRoom, int tEnter);
	int room_entered(int iRat, int iRoom);

	initTime = time(NULL);//Take a record of the start time

	//////////////POPULATE_ROOMS_FROM_FILE////////////
  	FILE *rooms = fopen("rooms", "r"); // Open file of rooms

  	if (rooms == NULL) {//If we can't access the file
    	fprintf(stderr, "\nOh no! It seems I can't read that file.");
  	}
  	int i;//These variables are for reading in from the room
  	int roomCap;
  	int roomTime;
  	for(i = 0;fscanf(rooms, "%d %d", &roomCap, &roomTime) != EOF; i++)//For each line of the data file
  	{
  		roomData[i][0]=roomCap;//Record the room capacity
  		roomData[i][1]=roomTime;//Record the room's time to cross
  		idealTime+=roomTime;//Add up the ideal times for the rooms
  		roomCount++;//Add one to the total number of rooms
  	}
	/////////////////////////////////////////////////////////
	if(argc != 3)//If we get the wrong number of arguments
		printf("%s\n", "Please make sure you give two arguments!");
	else
	{
		/////////////////////////////////////////////IN_ORDER_ALG/////////////////////////////////////////////////////////
		if(!strncmp("i", argv[2], 1))//In Order Algorithm mode
		{
			InOrder = 1;
		}
		else if(!strncmp("d", argv[2], 1))//Distributed Algorithm mode
		{
			InOrder = 0;
		}
		else if(!strncmp("n", argv[2], 1))//Nonblocking mode
		{
			Nonblocking = 1;
		}

		else//Not a correct algorithm selected
			printf("%s\n", "That's not an acceptable setting for the algorithm.");

		pthread_t idrat_0, idrat_1, idrat_2, idrat_3, idrat_4;//Create the threads for the rats
		pthread_t ratID[MAXRATS] = {idrat_0, idrat_1, idrat_2, idrat_3, idrat_4};//Create an array of those rats

		int roomsToMake;//Variablefor the for loop that makes rooms
		for(roomsToMake = 0; roomsToMake < roomCount; roomsToMake++)
		{
			sem_init(roomSems[roomsToMake], 0,roomData[roomsToMake][0]);//Initialize the room 0 semaphore to be shared by all threads, starting at a count of the room capacity.
		}

		int ratsToMake;//Variable for the loop of rats to make
		for(ratsToMake = 0; ratsToMake <atoi(argv[1]); ratsToMake++)
		{
			pthread_create(&(ratID[ratsToMake]), NULL, ratrun, (void *)ratsToMake);//Create the thread of rat i, using default attributes and passing it a number to be it's ID.
			runningRats++;//Incrementthe number of rat threads we have runnig.
		}
		while(runningRats);//While there are rats running, wait.

		//for each room, destroy the associated semaphore..
		int semsToDelete;
		for(semsToDelete = 0; semsToDelete < semsToDelete; semsToDelete++)
	    	(void)sem_destroy(roomSems[semsToDelete]);
	}

	int iR;//Itterator for the room.
	int enCnt;//Entry counter for itterative printing
	for(iR = 0; iR < roomCount; iR++)//Print the requested info.
	{
		printf("Room %d : [%d %d] : ", iR, roomData[iR][0], roomData[iR][1]);//Print the room info
		for(enCnt  = 0; enCnt < atoi(argv[1]); enCnt++)
		{
			printf(" %d %d %d; ", RoomVB[iR][enCnt].iRat ,RoomVB[iR][enCnt].tEntry, RoomVB[iR][enCnt].tDep + RoomVB[iR][enCnt].tEntry);//Print the info from that room's log book.
		}
		printf("\n");
	}

	printf("Total Traversal time: %d. Ideal time: %d. \n", totalTime, idealTime*atoi(argv[1]));//Print the total time rats spent traveling vs the ideal time.
}


/**
 *@param iRoom The room number we're writing in the log of.
 *@param iRat ID of rat
 *@param tEntry Entry time of rat
 *@param tDep Exit time of rat.
*/
void writeLog(int iRoom, int iRat, int tEnter, int tDep)
{
	RoomVB[iRoom][iRat].iRat = iRat;
	RoomVB[iRoom][iRat].tEntry = tEnter;
	RoomVB[iRoom][iRat].tDep = tDep;
}


/* iRat - id of rat leaving the room */
/* iRoom - id of room being left */
/* tEnter - time Rat entered room */
void LeaveRoom(int iRat, int iRoom, int tEnter)
{
	sem_post(roomSems[iRoom]);//Leaves the room, increments the room counter.
	int tDep = (time(NULL) - initTime) - tEnter;
	writeLog(iRoom, iRat, tEnter, tDep);
} 

/* iRat - id of rat entering the room */ /* iRoom - id of room being entered */
void EnterRoom(int iRat, int iRoom)
{
	sem_wait(roomSems[iRoom]);//Enters the room, decrements room counter.
	int time_entered = time(NULL)-initTime;//Tak record of the time entered.
	sleep(roomData[iRoom][1]);//waits for the time associated with this room.		
	LeaveRoom(iRat, iRoom, time_entered);   //Leave the room 
}

/*Try to enter the room. If the room is full, return -1. Else, return 0.*/
int TryToEnterRoom(int iRat, int iRoom)
{
	sem_getvalue(roomSems[iRoom], &value);
	if(value > 0)
	{
		EnterRoom(iRat, iRoom);//Travel through the room and record in the log
		roomAccessed[iRoom][iRat] = 1;//Record that we've entered that room
		return 0;
	}
	else
		return -1;
}

/*Return -1 if we've never entered this room, 0 if we have.*/
int room_entered(int iRat, int iRoom)
{
	if(roomAccessed[iRoom][iRat] == 1){return 0;}//If we've been to this room before
	return -1;
}

void *ratrun(void *arg)
{
	int ratStartTime = time(NULL) - initTime;//Take record of rat's start time
	int currentRoom = 0;//Initial room
	int rat_id = (int)arg;//Rat ID
	int startRoom;
	if(!Nonblocking)//If we're not in nonblocking mode
	{
		if(InOrder)//If we're going 0->n room order
		{
			for(currentRoom = 0; currentRoom < roomCount; currentRoom++)//For each room
			{
				EnterRoom(rat_id, currentRoom);//Enter the room. Will wait until empty if we have to.
			}
		}

		else//If we're in distribued rat mode
		{
			currentRoom = rat_id%roomCount;//urrent room is mod, incase rats>rooms
			startRoom = currentRoom;//startroom init
			int roomsVisited = 0;
			for(;roomsVisited<roomCount;currentRoom++)//Itterate through the mod ring of rooms
			{
				EnterRoom(rat_id, currentRoom%roomCount);//Go to the room
				roomsVisited++;//Keep track that we've visited it.
			}
		}
	}
	else//We're nonblocking. We will use the distributed start algorithm as it creates a more even spread of rats, helping to prevent bottlenecking at low-capacity rooms.
	{
		currentRoom = rat_id%roomCount;//Use the same init as the distributed mode
		startRoom = currentRoom;
		int roomsVisited = 0;
		while(roomsVisited != roomCount)//While this rat hasn't visited every room
		{
			for(;roomsVisited<roomCount;currentRoom++)//Itterate through the modular ring of rooms
			{
				if (currentRoom>=roomCount)//If we're at the n+1'th room, loop back to the start of the n-modular ring start
				{
					currentRoom = 0;//Go back to start of ring
				}
				if(room_entered(rat_id, currentRoom) == 0)//If we've entered the room
				{}//Don't try to go back in
				else//If we haven't entered the ring,
				{
					if(TryToEnterRoom(rat_id, currentRoom%roomCount) == 0)/*If we haven't entered and we can get in */ 
						/*the room, we will enter it. If we can't, we will go to another room. (Or try over and over if it's the last room.)*/
					{
						roomsVisited++;//Increment the totaol rooms visited count
					}
				}
			}
			//printf("%s\n", "While never ends");
		}

	}

	totalTime += (int)((time(NULL) - initTime) - ratStartTime);
	printf("Rat %d completed the maze in %d seconds. \n", rat_id, (int)((time(NULL) - initTime) - ratStartTime));
    runningRats--;
    pthread_exit(0);
}       
