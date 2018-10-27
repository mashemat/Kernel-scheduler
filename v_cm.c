#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include "papi.h"
#include <pthread.h>


#define MyRunTime 40
#define MATRIX_INIT 0
#define MX 100000

//Global variables
int execution_status=1;
char M[MX][MX];

//long_long op;
//double *ad[MX];

//The thread waits MyRunTime and set to zero execution_status in order to make the for exit
void *thread_run(){
	sleep(MyRunTime);
	execution_status = 0;
}

int main (int argc, char **argv) 
{
  int iter, i, j;
  char readChar;
  pthread_t th;
  int events[2] = {PAPI_L2_DCM, PAPI_FP_OPS }, ret;
  long_long values[2], values_init[2];



/* char **M = (char **)malloc(MX);
for(int i = 0; i < MX; i++) M[i] = (char *)malloc(MX);*/

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

pthread_create(&th, NULL, thread_run, NULL);

if ((ret = PAPI_read_counters(values_init, 2)) != PAPI_OK) {
    fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
    exit(1);
}



  //reading 
    for(iter = 0; execution_status; iter++) 
    {
      for(j=0; j<MX; j++)
        for(i=0; i<MX; i++){
          readChar= M[i][j];
 //         op++;
        }

    }

 if ((ret = PAPI_read_counters(values_init, 2)) != PAPI_OK) {
    fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
    exit(1);
}

printf("Cache miss: %lld\n", values_init[0]);   



  return 0;

}
