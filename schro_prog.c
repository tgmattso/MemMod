//
//  Schrodingers racy program ... is the cat dead or alive?
//
//  WARNING: This program has a data race and is therefore invalid.
//           But it works everywhere we've tested it so screw the
//           ANSI standard and its refusal to appreciate the usefulness
//           of benign data races!
//
//  History: Written by Tim Mattson, Feb 2019
//
#include <sys/time.h>
#include <omp.h>

// random number generator parameters
#define MULT 1366
#define ADD  150889
#define MOD  714025

#define NTRIALS 1000

// Cat states 
#define DEAD   0
#define ALIVE  1

// seed the pseudo random sequence with time of day
void seedIt(int *val)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *val = (int)tv.tv_usec;
}

// Linear congruential random number generator
int nextRan(int last)
{
   int next;
   next = (int) (MULT*last+ADD)%MOD;
   return next;
}

// wait a short random amount of time
double waitAbit()
{
   double val= 0.0;
   int i, count, rand;
   seedIt(&rand);
   count = nextRan(rand);
  
   // do some math to make us wait a while
   for (i = 0;     i<count; i++){
      rand = nextRan(rand);
      val += (double)rand/((double)MULT);
   }
   
   return val;
}

int main()
{
   double wait_val;
   int rand,i; 
   int dead_or_alive;

   for(i=0; i<NTRIALS; i++){
   #pragma omp parallel num_threads(2) shared(dead_or_alive)
   {
       if(omp_get_thread_num() == 0)
       {
         printf(" with %d threads\n",omp_get_num_threads());
         printf("Schrodingers program says the cat is ");
       }
       
       #pragma omp single
       {
          // these tasks are participating in a data race, but 
          // the program logic works fine if the actual value 
          // is messed up since in C any int other than 1 is false
          #pragma omp task
          {
              double val = waitAbit();
              // a store of a single machine word
              #pragma omp atomic write
              dead_or_alive = DEAD;
          }
          #pragma omp task
          {
              double val = waitAbit();
              // a store of a single machine word
              #pragma omp atomic write
              dead_or_alive = ALIVE;
          }
       }
   }
   if(dead_or_alive == ALIVE)
       printf(" alive. %d\n",(int)dead_or_alive);
   else 
       printf(" dead. %d\n",(int)dead_or_alive);
   } // end loop over trials (for testing only)
   
   return 0;
}
