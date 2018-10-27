#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include "papi.h"

#define NITER 20
#define MEGA 1000000
#define TOT_FLOPS (2*MX*MX*NITER)
#define RUN 1
#define MATRIX_INIT 0

//Global variables
int execution_status=1;
int MX = 0;
int runtime = 0;
int events[2] = {PAPI_L2_DCM, PAPI_FP_OPS }, ret;
long_long values[2];
long_long op;
//double *ad[MX];

/* Get actual CPU time in seconds */
float gettime() 
{
return((float)PAPI_get_virt_usec()*1000000.0);
}

void ALRMhandler (int sig)
{
  static long sum_cm=0;
  static int t=0;
  if ((ret = PAPI_read_counters(values, 2)) != PAPI_OK) {
    fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
    exit(1);
  }
  sum_cm+=values[0];

  printf("Cache misses: %ld\n", sum_cm);
  printf("Number of operations %lld\n", op);

  /* if ((ret = PAPI_start_counters(events, 2)) != PAPI_OK) {
     fprintf(stderr, "PAPI failed to start counters: %s\n", PAPI_strerror(ret));
     exit(1);
  }*/
  alarm(12);
  t++;
  if (t==5)
   execution_status = 0;
}

int main (int argc, char **argv) 
{

  float t0, t1;
  int iter, i, j, iteration;
  char readChar;
  char fname[100];
  FILE *fl;

  if(argc != 2){
    printf("Wrong number of parameters\n");
    exit(-1);
  }

  /* Take matrix dimensions */
  MX = atoi(argv[1]); 

 // char * * M = malloc(MX * MX);


  char **M = (char **)malloc(MX);
for(int i = 0; i < MX; i++) M[i] = (char *)malloc(MX);
  //char M[MX][MX];

  signal (SIGALRM, ALRMhandler);

  if (PAPI_num_counters() < 2) {
     fprintf(stderr, "No hardware counters here, or PAPI not supported.\n");
     exit(1);
  }

  /*--/*Matrix initialization*/
 //   for(i=0; i<MX; i++)
 //     for(j=0; j<MX; j++)
  //      M[i][j]=MATRIX_INIT;



  alarm (12);

  t0 = gettime();
  if ((ret = PAPI_start_counters(events, 2)) != PAPI_OK) {
     fprintf(stderr, "PAPI failed to start counters: %s\n", PAPI_strerror(ret));
     exit(1);
  }


  op = 0;
  //reading 
    for(iter = 0; execution_status; iter++) 
    {
      for(j=0; j<MX; j++)
        for(i=0; i<MX; i++){
          readChar=M[i][j];
          op++;
        }
    }


  if ((ret = PAPI_read_counters(values, 2)) != PAPI_OK) {
    fprintf(stderr, "PAPI failed to read counters: %s\n", PAPI_strerror(ret));
    exit(1);
  }
  t1 = gettime();

  /*printf("Total software flops = %f\n",(float)TOT_FLOPS);
  printf("Total hardware flops = %lld\n",(float)values[1]);
  printf("MFlop/s = %f\n", (float)(TOT_FLOPS/MEGA)/(t1-t0));
  printf("%lld\n", values[0]);*/



  /*
  //file name creation
  strcpy(fname, "pm_h_");
  strcat(fname, argv[1]);
  strcat(fname, "_");
  strcat(fname, argv[2]);
  strcat(fname, ".txt");

  fl = fopen(fname, "w+");

  //file write
  fprintf("%lld\n", values[0]);

  fclose(fl);*/


  return 0;


}  
