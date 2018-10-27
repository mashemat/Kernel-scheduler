#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include "papi.h"
#include <pthread.h>
/*Low level libraries*/
#include <unistd.h> //For getuid(),pipe() and others...
#include <sys/types.h> //For process functions (fork...)
#include <sys/wait.h> //For wait macros
#include <sys/stat.h> //For permissions, FIFO
#include <signal.h> //For signals like kill
#include <errno.h> //In order to see errno value
//#include <fcntl.h> //For manypulate file descriptors
//#include <sys/ipc.h> //For interprocess communication
#include <sys/shm.h> //For shared memory functions
//#include <sys/msg.h> //For message queues
//#include <pthread.h> //For thread management
//#include <semaphore.h> //For thread mutex

/*Standard libraries*/
#include <time.h> //For system time (srand(TIME(NULL)))
#include <math.h> //For math functions (like pow)

#define MyRunTime 10
#define MATRIX_INIT 0
/*State MACROS*/
#define IDLE 0
#define RUN 1
#define DONE 2
#define STOP 3

#define SHMSZ 100
#define EXIT_FAIL 1

#define MX 1000


//Global variables
int *execution_status; /*Status of the analyzed processes*/
char M[MX][MX];

//long_long op;
//double *ad[MX];

//The thread waits MyRunTime and set to zero execution_status in order to make the for exit
void *thread_run(){
	sleep(MyRunTime);
	execution_status = 0;
}

long_long matrix_reading(){

  int i, j, iter, events[2] = {PAPI_L2_DCM, PAPI_FP_OPS }, ret;
  long_long values[2], values_init[2];
  char readChar;

    /*--/*Matrix initialization*/
  for(i=0; i<MX; i++)
    for(j=0; j<MX; j++)
      M[i][j]=MATRIX_INIT;

  if (PAPI_num_counters() < 2) {
     fprintf(stderr, "No hardware counters here, or PAPI not supported.\n");
     exit(1);
  }

  if ((ret = PAPI_start_counters(events, 2)) != PAPI_OK) {
     fprintf(stderr, "PAPI failed to start counters: %s\n", PAPI_strerror(ret));
     exit(1);
  }
//wait for synch
  while (*execution_status!=RUN); /*Wait for RUN*/

	if ((ret = PAPI_read_counters(values_init, 2)) != PAPI_OK) {
    fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
    exit(1);
  }

  //reading 
    for(iter = 0; *execution_status == RUN; iter++) 
    {
      	for(j=0; j<MX; j++){
	        for(i=0; i<MX; i++){
	          readChar=M[i][j];
	        }
    	}
    }

 if ((ret = PAPI_read_counters(values_init, 2)) != PAPI_OK) {
    fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
    exit(1);
  }

    while (*execution_status!=STOP); /*Wait for STOP*/
    
    return values_init[0];
}
int main (int argc, char **argv) 
{
  int i, j;
  long_long cache_miss;

  
/*--/*For shared memory*/
	int shmid;

  /*--/*Create shared memory*/
	if ((shmid=shmget(IPC_PRIVATE, SHMSZ, IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR))<0)
	{
		perror("Error in shmget: ");
		exit(EXIT_FAIL);
	}
/*--/*Attach to the process*/
	if ((execution_status = shmat(shmid, NULL, 0)) == (int *) -1)
	{
		perror("Error in shmat: ");
		exit(EXIT_FAIL);
	} 

//create child
  int PID = fork();
//child function
  if(PID == 0){

  	cache_miss = matrix_reading();

	printf("Cache miss: %lld\n", cache_miss);

	exit(1); 
  }

  /*--/*Parent handle the child synchronization*/
	for(i=0; i<1; i++)
	{
		*execution_status=IDLE;
		printf("Start run #%d\n", i+1);
		sleep(2);
		*execution_status=RUN;
		sleep(MyRunTime);
		*execution_status=DONE;
		sleep(2);
		*execution_status=STOP;
		sleep(2);
	}  



  return 0;

}