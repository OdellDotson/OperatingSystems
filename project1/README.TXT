-------------------------------------------------------------------------------
Operating Systems  -Project 3
-------------------------------------------------------------------------------
Credits
	Odell Dotson - ocdotson
-------------------------------------------------------------------------------
Project description

This project simulates rats moving through rooms of different capacities and lengths.
To run the project, first follow the installation instructions.
Next, 
-------------------------------------------------------------------------------
Dependencies:

pthread
-------------------------------------------------------------------------------
Installation and running instructions

Unzip the archive using whatever unzipping method you would like, and then cd into the directory.
Run "make" (without quotes) in the directory of the zip file. This will create an executable file.



This is the command to run:
./maze [number of rats] [type of algorithm]
where number of rats is a number between 1 and 5. This selects how many rats there will be.

Type of algorithm can be i, d  or n.
i selects the In Order algorithm. The rats will move through the rooms starting at room 0, and ending at the last room. 
	If the room is full, the rat will wait until it is free before moving into it.
d selects Distributed algorithm. Each rat will start in the next room from the previous one, and the final room wraps around in a modular ring.
n selects Non-Blocking mode. Rats will not wait for other rats to finish in a room, rather skipping fowards until they find a free room.

-------------------------------------------------------------------------------