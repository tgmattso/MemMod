/*
**  PROGRAM: A simple SPMD pipeline code wrapped around
**           the producer/consumer program to measure 
**           overheads in different sycn approaches.
**
**  HISTORY: Written by Tim Mattson, Dec 2018.
*/
#include "omp.h"
#ifndef APPLE
#include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define Niters   10000000
#define Ntries   100
#define Nthreads 2
#define BIG      100000000000.0  // larger than any time I'd measure
#define TINY     0.0000000001    // smaller than any time I'd measure
//#define DEBUG    1


// function to compute a standard deviation from sum 
// of values (sumxi) and sum of values sqaured (sumxisq)
double stdev(int N, double sumxi, double sumxisq)
{
   double ave, stdev;
   if(N<2) return -1.0;  // must have 2 or more samples

   ave = sumxi/(double)N;
   stdev = (double)N*ave*ave + sumxisq - 2*ave*sumxi;
   stdev = stdev/(double)(N-1);

   return sqrt(stdev);
}

int main()
{
  double rtsum=0.0, rtsqsum=0.0, rtmin=BIG, rtmax=TINY, rtsdev;  
  double runtime0, runtime1;
  double *A, sumCons=0, sumProd=0;
  int numthreads, ierr=0, flag = -1;

  omp_set_num_threads(Nthreads);

  // A an array of values
  A = (double*)malloc(Niters*sizeof(double));

  #pragma omp parallel shared(numthreads, flag, A, sumProd, sumCons)
  {
    int id = omp_get_thread_num();
    int itries, iters, flag_temp;
    double runtime, runstart;

    // Exit if we didn't get the two threads we asked for
    if(id==0){
       numthreads = omp_get_num_threads();
       if(numthreads != 2) {
          printf("error: incorect number of threads, %d \n",numthreads);
          exit(-1);
       }
    }
    #pragma omp barrier

    for (itries=0;itries<Ntries;itries++){

       // setup flags and sums for next try
       #pragma omp single
       {
         flag = -1;
         sumProd = 0.0;
         sumCons = 0.0;
       }

       //  the producer thread
       if(id == 0){
          runtime0 = omp_get_wtime();
          for(iters = 0; iters<Niters; iters++){
             *(A+iters) = (double)iters;
             #pragma omp flush
                ++flag;
             #pragma omp flush (flag)
             sumProd += *(A+iters);
          }
       }

       // The consumer thread
       if(id == 1) {
          runtime1 = omp_get_wtime();
          for(iters = 0; iters<Niters; iters++){
             #pragma omp flush (flag)
             while(1){
            #pragma omp flush (flag)
                flag_temp=flag;
                if(flag_temp >= iters)break;
             }
             #pragma omp flush 
             sumCons += *(A+iters);
          }

          // collect timing statistics on the consumer
          // since it has to finish last. 
          runtime = omp_get_wtime();
          runstart=((runtime0<runtime1)?runtime0:runtime1);
          runtime = runtime-runstart;
          if(runtime > rtmax) rtmax = runtime;
          if(runtime < rtmin) rtmin = runtime;
          rtsum   += runtime;
          rtsqsum += runtime*runtime;
          #ifdef DEBUG
             printf("%d, runtime=%f, max=%f, min=%f, rtsum=%f, rtsqsum=%f\n",
                    itries,runtime,rtmax,rtmin,rtsum, rtsqsum);
          #endif
       }  
       #pragma omp barrier

       // test results 
       #pragma omp master
       {
          if (sumProd != sumCons){
            printf(" sums don't match: Prod=%d, Cons=%d\n",sumProd, sumCons);
            ierr++;
          }
       }
    }
  }
  printf("\n Old flush only, no atomics\n");
  printf("Pipeline done with %d threads and %d errors \n",numthreads,ierr);
  rtsdev = stdev(Ntries, rtsum, rtsqsum);

  printf("%d trials, ave=%f, stdev=%f, min=%f, max=%f\n", (int)Ntries,
        (float)(rtsum/Ntries), (float)rtsdev, (float)rtmin, (float)rtmax);
}
 
