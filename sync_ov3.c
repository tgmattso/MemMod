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
#include <stdatomic.h>

#define Niters   10000000
#define Nthreads 2

int main()
{
  double runtime;
  int *A, sumCons=0, sumProd=0;
  int numthreads, flag = -1;

  omp_set_num_threads(Nthreads);

  // A an array of values
  A = (int*)malloc(Niters*sizeof(double));

  runtime = omp_get_wtime();
  #pragma omp parallel shared(numthreads, flag, A, sumProd, sumCons)
  {
    int id = omp_get_thread_num();
    int iters, flag_temp;

    // Exit if we didn't get the two threads we asked for
    if(id==0){
       numthreads = omp_get_num_threads();
       if(numthreads != 2) {
          printf("error: incorect number of threads, %d \n",numthreads);
          exit(-1);
       }
    }
    #pragma omp barrier

    //  the producer thread
    if(id == 0){
       for(iters = 0; iters<Niters; iters++){
          *(A+iters) = iters;
//  Deepack suggested the following atomi_fetch_add_explicit()
//          #pragma omp atomic release
//             ++flag;
//atomic_fetch_add_explicit((atomic_int_t *)&flag, 1,memory_order_release);
atomic_fetch_add_explicit((atomic_int *)&flag, 1,memory_order_release);
          sumProd += *(A+iters);
       }
    }

    // The consumer thread
    if(id == 1) {
       for(iters = 0; iters<Niters; iters++){
          while(1){
//  Deepack suggested the following atomi_fetch_add_explicit()
//             #pragma omp atomic read acquire
//                 flag_temp=flag;
//flag_temp = atomic_load_explicit((atomic_int_t *)&flag, memory_order_acquire);
flag_temp = atomic_load_explicit((atomic_int *)&flag, memory_order_acquire);
             if(flag_temp >= iters)break;
          }
          sumCons += *(A+iters);
       }
    }  
  }
  runtime = omp_get_wtime() - runtime;
  if (sumProd != sumCons)printf(" sums don't match: Prod=%d, Cons=%d\n",
          sumProd, sumCons);

  printf("\n C11 release-acquire, no flushes\n);
  printf("Pipeline done with %d threads in %lf secs",numthreads,runtime);
  printf(" SUMS:  Prod=%d, Cons=%d \n", sumProd, sumCons);

}
 
