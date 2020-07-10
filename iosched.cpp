//Stephanie Michalowicz
//Operating Systems
//Lab 4 - I/O Scheduler

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <vector>
#include <cstring>
#include <sstream>
#include <locale>				
#include <algorithm>
#include <iomanip>
#include <unistd.h>
#include <ctype.h>
#include <queue>
#include <list>
#include <iterator>

using namespace std; 

//file name
ifstream the_file;

//stream variables
char line[1024];
int time_to_issue = 0;
int requestedTrack = 0;


//create a structure for i/o operations
struct io_operation {

	int operationNum;
    int arrival;     
    int track;      
    int issue_time;
    int finish_time; 

};

//create a queue that holds all incoming io requests
vector<io_operation> io_queue; 

//generic vector
vector <io_operation*> generic_vector;

//current time variables
io_operation * current_io = NULL;
int currentTrack = 0;
int now = 0;
int queue_index = 0; 				//keep track of place in io_queue
int num_ios = 0;			        //number of io requests in the io_queue
int currentDirection = 1;  			//define current direction -1 = down, 1 = up   // start by going up
int direction;


//current operation stats variables
int movement = 0;						//number of tracks the head moved from issue to completion time
double turnaround = 0;					//submission to compeletion time
double waitTime = 0;					//submission to issue time

//main stats variables
int tot_movement = 0;
double total_turnaround = 0;
double totalWaitTime = 0;

double avg_turnaround = 0;
double avg_waittime = 0;
int max_waittime = 0;



void get_instructions(){

	while(the_file.good()){
		//read the line and stick contents into line var
		the_file.getline(line, 1024);

		//skip lines with comments
		if((line[0] == '#') or (line[0] == '\0') ){
			continue;
		}else{

			//store the issue time and requested track
			string sLine = line;
			std::istringstream stream(sLine);
			stream >> time_to_issue >> requestedTrack;

			//create a temporary io_operation for storage of requests
			io_operation io_temp;

			//assign values each io_operation structure
			io_temp.operationNum = num_ios;
			io_temp.arrival = time_to_issue;
			io_temp.track = requestedTrack;
			io_temp.finish_time = 0; 

			//preload io_queue with ALL I/O requests
			io_queue.push_back(io_temp);

			//count # of operations
			num_ios++;

		}	
	}
}

//generic scheudler class
class Scheduler{

public:
	virtual io_operation * getNext() = 0;

	virtual void add(io_operation * op){
		//add io operation
		(generic_vector).push_back(op);	
	}

	virtual void finish(){
		//calculate and assign track movement
		movement = (now - current_io->arrival);		

		//assign turnaround time
		turnaround = (now - current_io->arrival);		

		//add current turnaround time to total turnaround time
		total_turnaround += turnaround;

		//complete disk request, assign finish time to io operation
		current_io->finish_time = now;
	}

	virtual void issue(){
		//assign arrival and issued time
		current_io->issue_time = now;

		//assign current wait time
		waitTime = (current_io->issue_time - current_io->arrival); 
		 
		//add current wait time to total wait time
		totalWaitTime += waitTime;

		//figure out max wait time
		if (max_waittime < waitTime){
			max_waittime = waitTime;
		}
	}

};

//Global Scheduler Variable 
Scheduler * scheduler;

//////////////////////////////////First In First Out Scheduler (first come first serve)///////////

class FIFO : public Scheduler{

public:
	io_operation * getNext();
}; 

///////////////////Define FIFO functions

io_operation * FIFO :: getNext(){

	io_operation * opPointer;
		
	if (generic_vector.empty()){
		opPointer = NULL;
	}else{
		opPointer = generic_vector.front();				//return address to opPointer	
 		generic_vector.erase(generic_vector.begin());  //erase first element in the FIFO vector
	}

 	return opPointer;
}


//////////////////////////////////Shortest Seek Time First Scheduler/////////////////////////////

class SSTF : public Scheduler{

public:
	io_operation * getNext();
}; 

/////////////////Define SSTF functions

io_operation * SSTF :: getNext(){

	io_operation * opPointer;

	if (generic_vector.empty()){
		opPointer = NULL;
		return opPointer;
	}else if (generic_vector.size() == 1){	
		opPointer = generic_vector.front();					//return address to opPointer
		generic_vector.erase(generic_vector.begin());		//erase first element in the SSTF vector
		return opPointer;

	}else{

		//shortest seek time variable
		int SST = abs(currentTrack - generic_vector[0]->track);

		//SSTF vector index
		int SSTF_index = 0;

		for(int i = 0; i < generic_vector.size(); i++){

			//if track request is the same as current track
			if (SST == 0){
				current_io = generic_vector[SSTF_index];
			}

			//if new seek time is shorter than current shortest seek time
			if ( abs(currentTrack - generic_vector[i]->track) < abs(SST) ){
				SST = abs(currentTrack - generic_vector[i]->track);
				SSTF_index = i;
			}
		}

		opPointer = generic_vector[SSTF_index];     				
		generic_vector.erase(generic_vector.begin() + SSTF_index);	 //erase that element from the SSTF vector by using the SSTF vector bindex

	}

 	return opPointer;
}


//////////////////////////////////LOOK Scheduler (elevator)//////////////////////////////////////

class LOOK : public Scheduler{


public:
	io_operation * getNext();

}; 

/////////////////Define LOOK functions

io_operation * LOOK :: getNext(){     

	io_operation * opPointer = NULL;

	//shortest seek time variable
	int SST = 100000000;

	// LOOK vector index
	int LOOK_index = -1;

	for (int k = 0; k < 2 ; k++) {

		//for every item in the active queue, look ahead to find the next closest track heading in the same direction
		for(int i = 0; i < generic_vector.size(); i++){

            direction = currentDirection;

			// determine direction
			if (generic_vector[i]->track < currentTrack){direction = -1;}  //direction is going down
			if (generic_vector[i]->track > currentTrack){direction = 1;}  //direction is going up

			//skip this io operation if its not going in the same direction
			if (direction != currentDirection){
				continue;
			}

			//assign current operation as the shortest seek time and mark its position
			if(abs(currentTrack - generic_vector[i]->track) < SST){
				SST = abs(currentTrack - generic_vector[i]->track);
				LOOK_index = i;
			}
		}

		if (LOOK_index != -1 ) {
			// done
			opPointer = generic_vector[LOOK_index];							
			generic_vector.erase(generic_vector.begin() + (LOOK_index));	 //erase that element from the LOOK vector by using the LOOK vector bindex

		 	return opPointer;
		}

		currentDirection *= -1;
	}

	return opPointer;
}

//////////////////////////////////Circular LOOK Scheduler///////////////////////////////////////


class CLOOK : public Scheduler{

public:
	io_operation * getNext();
}; 

io_operation * CLOOK :: getNext(){

	io_operation * opPointer = NULL;

	//shortest seek time variable
	int SST = 100000000;
	int lowest_track = 10000000;

	// LOOK vector index
	int LOOK_index = -1;
	int LOOK_lowest = -1;

	for (int k = 0; k < 3; k++){
		//for every item in the active queue, look ahead to find the next closest track
		for(int i = 0; i < generic_vector.size(); i++){

            direction = currentDirection;

			// determine direction
			if (generic_vector[i]->track < currentTrack){direction = -1;}  //direction is going down
			if (generic_vector[i]->track > currentTrack){direction = 1;}  //direction is going up
            if (generic_vector[i]->track == 0){direction = 0;}

			if(direction != currentDirection) {

				if (generic_vector[i]->track < lowest_track) { 
					lowest_track = generic_vector[i]->track;
					LOOK_lowest = i;
				}
                
				continue;
			}
 
			//assign current operation as the shortest seek time and mark its position
			if((direction == 1) and (abs(currentTrack - generic_vector[i]->track) < SST)){
				SST = abs(currentTrack - generic_vector[i]->track);
				LOOK_index = i;
			}

		}



		if ((LOOK_index != -1) and (currentDirection == 1)){
			// done
			opPointer = generic_vector[LOOK_index];
			generic_vector.erase(generic_vector.begin() + (LOOK_index));	 //erase that element from the LOOK vector by using the LOOK vector bindex
			return opPointer;
		}

		if ((LOOK_lowest != -1 ) and (currentDirection == -1)){
			opPointer = generic_vector[LOOK_lowest];							//store value of operation with shortest seek time in global variable 
			generic_vector.erase(generic_vector.begin() + (LOOK_lowest));	 //erase that element from the LOOK vector by using the LOOK vector bindex
		 	return opPointer;

		}

		if(currentDirection == 0){
			currentDirection = 1;
			opPointer = generic_vector[LOOK_lowest];
			generic_vector.erase(generic_vector.begin() + (LOOK_lowest));
			return opPointer;
		}
        
		currentDirection *= -1;
	}
    
	return opPointer;
}


//////////////////////////////////FLOOK Scheduler///////////////////////////////////////////////

class FLOOK : public Scheduler{

	//define a private vector for the FLOOK scheduler
	vector <io_operation*> Q[2];

	vector <io_operation*> * q_add;
	vector <io_operation*> * q_read;

public:

	FLOOK(){
		q_add = &Q[0];	
		q_read = &Q[1];
	}

	void add(io_operation * op);
	io_operation * getNext();
};

void FLOOK :: add(io_operation * op){

	//add io operation to FIFO vector
	(*q_add).push_back(op);

}

io_operation * FLOOK :: getNext(){

	io_operation * opPointer = NULL;

	//shortest seek time variable
	int SST = 100000000;

	// LOOK vector index
	int LOOK_index = -1;

	for (int j=0; j < 2; j++) {
		for (int k=0; k < 2 ; k++) {

			//for every item in the active queue, look ahead to find the next closest track
			for(int i = 0; i < q_read->size(); i++){

                direction = currentDirection;

				// determine direction
				if ((*q_read)[i]->track < currentTrack){direction = -1;}  //direction is going down
				if ((*q_read)[i]->track > currentTrack){direction = 1;}  //direction is going up

				if (direction != currentDirection) {
					continue;
				}

				//assign current operation
				if((direction == currentDirection) and abs(currentTrack - (*q_read)[i]->track) < abs(SST)){
					SST = abs(currentTrack - (*q_read)[i]->track);
					LOOK_index = i;
				}
			}

			if (LOOK_index != -1 ) {
				// done
				opPointer = (*q_read)[LOOK_index];    					
				(*q_read).erase((*q_read).begin() + (LOOK_index));		 //erase that element from the LOOK vector by using the LOOK vector bindex
			 	return opPointer;
			}

			currentDirection *= -1;
		}

		swap(q_add, q_read);
	}

	return opPointer;
}


/////////////////////////////////////////////////////////////////////

int main( int argc, char *argv[] ){

	int option;
	
	//read command line options for a scheduler 
		while ((option = getopt (argc, argv, "s:")) != -1){
				switch (option){
					//alogrithm options
			    	case 's':{
			        	switch(*optarg){
			        		case 'i':{
			        			scheduler = new FIFO();
			        			break;
			        		}case 'j':{
			        			scheduler = new SSTF();
			        			break;
			        		}case 's':{
			        	 		scheduler = new LOOK();
			        			break;
			        		}case 'c':{
			        		 	scheduler = new CLOOK();
			        			break;
			        		}case 'f':{
			        			scheduler = new FLOOK();
			        			break;
			        		} default:{
			        			scheduler = new FIFO();
			        			break;
			        		 }
						}
			        	break;
	      			}
			    }
		}

	//look for a file and open it
	
	the_file.open(argv[optind]);

	if( !the_file.is_open()){
		cout << "Can't open the file!" << endl;
	}else{
		//do something if the file is open

				get_instructions();
				
				while(current_io or (queue_index < num_ios)){

					//increment total simulation time by 1 
					now++;

					if((queue_index < num_ios) && (io_queue[queue_index].arrival == now)){

						io_operation * op = &io_queue[queue_index];
						scheduler->add(op);
						queue_index ++;
					}

					LOOP:{
						//check to see if we reached the requested track, if so FINISH current IO operation
						if (current_io && (current_io->track == currentTrack)){

							scheduler->finish();

							io_queue[current_io->operationNum] = *current_io;

							//reset current io
							current_io = NULL;
						}

						//if current_io is NULL, ISSUE another IO operation
						if (current_io == NULL){
							current_io = scheduler->getNext();
							
							if (current_io){
								scheduler->issue();
							}
						}

						if (current_io && (current_io->track == currentTrack)){
							goto LOOP;
						}
	
					}

					//move head position to next track
					if(current_io){
						if(current_io->track >= currentTrack){    
					 		tot_movement++;
							currentTrack++;
						}else if(current_io->track < currentTrack){	
							tot_movement++;
							currentTrack--;	
						}else{
					 		continue;
						}

					}

				}

				//print each i/o operation
				for(int i=0; i < io_queue.size(); i++){
					if (io_queue[i].operationNum == i){
						printf("%5d: %5d %5d %5d\n", io_queue[i].operationNum, io_queue[i].arrival, io_queue[i].issue_time, io_queue[i].finish_time);
					}
				}

				avg_turnaround = (total_turnaround / num_ios); 
				avg_waittime = (totalWaitTime / num_ios);

				//print out stats
				printf("SUM: %d %d %.2lf %.2lf %d\n", now, tot_movement, avg_turnaround, avg_waittime, max_waittime); 
	}
				
	return 0;
}
