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
#include <stdlib.h>
#include <stdio.h>
#include <time.h> //For system time (srand(TIME(NULL)))
#include <string.h> //For manipulating strings
#include <math.h> //For math functions (like pow)

/*Added libraries*/
#include "papi.h"

/*This program is a testbench used to evaluate the influence of cache miss rate using computed cache
miss from PAPI tool. The program creates a char matrix and then fork two types of processes:
- READ_HOR_PROCESSES: Read the matrix horizontaly
- READ_VERT_PROCESSES: Read the matrix vertically
According to the type of process we should obtain two different cache miss rates, since they perform
the reading in two different directions.
								CHILD PROGRAM FLOW
	- With access_pcb system call the cache miss rate is updated in the current process PCB
	- The two types of processes start the reading and stop it at the "same" time.
	- The number of occurred cache miss is retrieved by PAPI and used as input
		for the next iteration (for first cycle is set to 1)
	- Also the computation time is calculated, to be used as delta time 
*/

/*To compile with papi.h libraries in linux:
gcc our_testbench3.c -I /home/fabio/Desktop/DARIO,BENITO,SEBASTIANO/include -L /home/fabio/Desktop/DARIO,BENITO,SEBASTIANO/lib -lpapi	*/

/**************************************************************************************************/
/*Return MACROS*/
#define EXIT_SUCCESS 0
#define EXIT_FAIL 1
/*Matrix MACROS*/
#define NROWS 10
#define NCOLUMNS 10
#define MATRIX_INIT 0
/*State MACROS*/
#define IDLE 0
#define RUN 1
#define DONE 2
#define STOP 3
/*System call MACROS*/
#define SYS_ACCESS_PCB 327
#define SYS_RETRIVE_PCB 328
/*Processes MACROS*/
#define NGROUPS 2 //It works only for 2!!!!! don't change
#define NPROC_FOR_GROUP 1
/*Other MACROS*/
#define SHMSZ 100
#define NRUNS 2


//run time
//#define myRunTime 2
int myRunTime;

/*Structures defiinition*/
struct cache_stat_type
{
	long_long cm;
	double dt;
};

/*Global variables*/
int *execution_status; /*Status of the analyzed processes*/
char M[NROWS][NCOLUMNS]; /*Matrix definition*/

/*Function prototypes*/
/*Function performed by the READ_HOR_PROCESSES*/
struct cache_stat_type read_hor_function(struct cache_stat_type) ; 
/*Function performed by the READ_VERT_PROCESSES*/
struct cache_stat_type read_ver_function(struct cache_stat_type) ; 

void init_papi_counters();

int main (int argc, char ** argv) 
{

	myRunTime = strtol(argv[1], NULL, 10);

/*--/*For child creation*/
	pid_t child_pid[NGROUPS*NPROC_FOR_GROUP];
/*--/*For shared memory*/
	int shmid;
/*--/*For cache miss statistics*/
	struct cache_stat_type cache_stat;
/*--/*Others*/
	int iter, i, j;
	
	
/*--/*Cache statistics initialization*/
	cache_stat.cm=1;
	cache_stat.dt=1;
	
/*--/*Matrix initialization*/
	for(i=0; i<NROWS; i++)
		for(j=0; j<NCOLUMNS; j++)
			M[i][j]=MATRIX_INIT;

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
	
	*execution_status=IDLE; /*wait the creation of all processes*/
	
/*--/*Create the processes*/
	for(i=0; i<NGROUPS*NPROC_FOR_GROUP; i++)
	{
		child_pid[i]=fork();
		if(child_pid[i]==0) /*I'm the child*/
		{
/*----------/*Let the childs perform the matrix reading*/		
			if(i<NPROC_FOR_GROUP) /*READ_HOR_PROCESSES group*/
			{
				printf("Index: %d, PID=%d, HOR Process\n", i, getpid());
				fflush(stdout);
				init_papi_counters();
				for(j=0; j<NRUNS ; j++)
				{
					cache_stat=read_hor_function(cache_stat);

				}
				
/*--------------/*Before exiting restore PCB cache miss to 1 to avoid average cache miss increse*/
				cache_stat.cm=1;
				cache_stat.dt=1;
				cache_stat=read_hor_function(cache_stat);
				fflush(stdout);
				
				exit(EXIT_SUCCESS);
			}
			else /*READ_VERT_PROCESSES group*/
			{
				printf("Index: %d, PID=%d, VER Process\n", i, getpid());
				init_papi_counters();
				for(j=0; j<NRUNS ; j++)
				{
					cache_stat=read_ver_function(cache_stat);
				}
				
/*--------------/*Before exiting restore PCB cache miss to 1 to avoid average cache miss increse*/
				cache_stat.cm=1;
				cache_stat.dt=1;
				cache_stat=read_ver_function(cache_stat);
				
				exit(EXIT_SUCCESS);
			}
		}
    }
/*--/*Only the parent will execute this code*/

printf("\t myRunTime: %d\n", myRunTime);
//printf("\t myRunTime: %d\n",(int) argv[1]);

/*--/*Parent handle the child synchronization*/
	for(i=0; i<NRUNS+1; i++)
	{
		*execution_status=IDLE;
		printf("Start run #%d\n", i+1);
		sleep(myRunTime);
		//sleep((int)argv[1]);
		*execution_status=RUN;
		sleep(myRunTime);
		//sleep((int)argv[1]);
		*execution_status=DONE;
		sleep(myRunTime);
		//sleep((int)argv[1]);
		*execution_status=STOP;
		sleep(myRunTime);
		//sleep((int)argv[1]);
	}
	
/*--/*Wait all the children*/
	while(wait(NULL)>=0);
	
	return EXIT_SUCCESS;
}

void init_papi_counters()
{
	int events[2] = {PAPI_L2_DCM, PAPI_FP_OPS }, ret;

/*--/*Initializing PAPI*/
	if (PAPI_num_counters() < 2) {
	   fprintf(stderr, "No hardware counters here, or PAPI not supported.\n");
	   exit(EXIT_FAIL);
	}

/*--/*Start cache miss counter monitoring*/
	if ((ret = PAPI_start_counters(events, 2)) != PAPI_OK) {
		fprintf(stderr, "PAPI failed to start counters a: %s\n", PAPI_strerror(ret));
		exit(EXIT_FAIL);
	}
}

/*Function performed by the READ_HOR_PROCESSES*/
struct cache_stat_type read_hor_function(struct cache_stat_type cache_stat)
{
/*--/*For elapsed time evaluation*/
	clock_t start, end;
	double cpu_time_used;
	int delta_time;
/*--/*For PAPI cache miss evaluation*/
	int ret;
	long_long values_initial[2], values_final[2];
/*--/*Others*/
	int i,j,iteration;
	char readChar;
	struct cache_stat_type cache_stat_return;


	
/*--/*Update PCB information on cache miss*/
	delta_time= (int) (cache_stat.dt+0.5); /*+0.5 for Rounding up*/
	syscall(SYS_ACCESS_PCB, getpid(), cache_stat.cm, delta_time, NGROUPS*NRUNS);
	while (*execution_status!=RUN); /*Wait for RUN*/
	start = clock();
	
/*--/*Read back the cache miss*/
	if ((ret = PAPI_read_counters(values_initial, 2)) != PAPI_OK) {
		fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
		exit(EXIT_FAIL);
	}

/*--/*Read the matrix*/
    for(iteration=0; *execution_status==RUN; iteration++) 
	{
		for(i=0; i<NROWS; i++)
			for(j=0; j<NCOLUMNS; j++)
				readChar=M[i][j];
	}
	
/*--/*Get elapsed time*/
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	
/*--/*Read back the cache miss*/
	if ((ret = PAPI_read_counters(values_final, 2)) != PAPI_OK) {
		fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
		exit(EXIT_FAIL);
	}
	
/*--/*Update the cache miss statistics to be returned*/
	cache_stat_return.cm=values_final[0]-values_initial[0];
	cache_stat_return.dt=cpu_time_used*1000;

	
	printf("pid: %d, PAPI cache miss: %lld, PAPI delta time: %f\n", getpid(), cache_stat_return.cm, cache_stat_return.dt);
	printf("pid: %d, op: %d, cm: %ld, cm_avg: %ld, nr_running: %ld, co1: %ld, co2: %ld, co3: %ld\n",
			getpid(),
			iteration,
			syscall(SYS_RETRIVE_PCB,getpid(),5),
			syscall(SYS_RETRIVE_PCB,getpid(),4),
			syscall(SYS_RETRIVE_PCB,getpid(),6),
			syscall(SYS_RETRIVE_PCB,getpid(),1),
			syscall(SYS_RETRIVE_PCB,getpid(),2),
			syscall(SYS_RETRIVE_PCB,getpid(),3));
			
	fflush(stdout);
	
	while (*execution_status!=STOP); /*Wait for STOP*/
	return cache_stat_return;
}

/*Function performed by the READ_VER_PROCESSES*/
struct cache_stat_type read_ver_function(struct cache_stat_type cache_stat)
{
/*--/*For elapsed time evaluation*/
	clock_t start, end;
	double cpu_time_used;
	int delta_time;
/*--/*For PAPI cache miss evaluation*/
	int ret;
	long_long values_initial[2], values_final[2];
/*--/*Others*/
	int i,j,iteration;
	char readChar;
	struct cache_stat_type cache_stat_return;
	
/*--/*Update PCB information on cache miss*/
	delta_time= (int) (cache_stat.dt+0.5); /*+0.5 for Rounding up*/
	syscall(SYS_ACCESS_PCB, getpid(), cache_stat.cm, delta_time, NGROUPS*NRUNS);

	while (*execution_status!=RUN); /*Wait for RUN*/
	start = clock();
	
/*--/*Read back the cache miss*/
	if ((ret = PAPI_read_counters(values_initial, 2)) != PAPI_OK) {
		fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
		exit(EXIT_FAIL);
	}
	
/*--/*Read the matrix*/
    for(iteration=0; *execution_status==RUN; iteration++) 
	{
		for(j=0; j<NCOLUMNS; j++)
			for(i=0; i<NROWS; i++)		
				readChar=M[i][j];
	}
	
/*--/*Get elapsed time*/
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	
/*--/*Read back the cache miss*/
	if ((ret = PAPI_read_counters(values_final, 2)) != PAPI_OK) {
		fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
		exit(EXIT_FAIL);
	}
	
/*--/*Update the cache miss statistics to be returned*/
	cache_stat_return.cm=values_final[0]-values_initial[0];
	cache_stat_return.dt=cpu_time_used*1000;
	
	printf("pid: %d, PAPI cache miss: %lld, PAPI delta time: %f\n", getpid(), cache_stat_return.cm, cache_stat_return.dt);
	printf("pid: %d, op: %d, cm: %ld, cm_avg: %ld, nr_running: %ld, co1: %ld, co2: %ld, co3: %ld\n",
			getpid(),
			iteration,
			syscall(SYS_RETRIVE_PCB,getpid(),5),
			syscall(SYS_RETRIVE_PCB,getpid(),4),
			syscall(SYS_RETRIVE_PCB,getpid(),6),
			syscall(SYS_RETRIVE_PCB,getpid(),1),
			syscall(SYS_RETRIVE_PCB,getpid(),2),
			syscall(SYS_RETRIVE_PCB,getpid(),3));
			
	fflush(stdout);
	
	while (*execution_status!=STOP); /*Wait for STOP*/
	return cache_stat_return;
}