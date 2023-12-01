/*
  Not sure what this means in the random437.h file provided for the class
  #ifndef random437_h
  #define random437_h
*/

#include <stdio.h> /* Used in random437.h file */
#include <stdlib.h> /* Used in random437.h file */
#include <unistd.h>
#include <getopt.h>
#include <math.h> /* Used in random437.h file */
#include <semaphore.h>
#include <pthread.h>



#define MAXWAITPEOPLE 800

/*
  Add
*/

#define MAX_TIME 600
#define MAX_THREADS 10


sem_t ad_waitingLineMutex;
sem_t ad_carMutex;
sem_t ad_arrivalMutex;

int ad_waitingLine = 0;
int ad_totalArrivals = 0;
int ad_totalRideTakers = 0;
int ad_totalRejected = 0;

int CARNUM; // Number of Explorer cars

void *adWaitingLineThread(void *arg) {
    for(int t = 0; t < MAX_TIME; ++t) {
        sem_wait(&ad_waitingLineMutex);
        printf("%d arrive ", t);
        
        //simulate the arrival with using the poissonRandom function
        int ad_arrivals = poissonRandom(getMeanArrival(t));
        ad_totalArrivals += ad_arrivals;
     
        //check if the line's waiting space exists
        int ad_spaceAvailable = MAXWAITPEOPLE - ad_waitingLine;
    
        if(ad_arrivals <= ad_spaceAvailable) {
            ad_waitingLine += ad_arrivals;
            printf("%d reject 0 wait-line %d at %02d:%02d:%02d\n", ad_arrivals, ad_waitingLine, t/60, t%60,0);
        } else {
            int ad_rejected = ad_arrivals - ad_spaceAvailable;
    	    ad_totalRejected += ad_rejected;
    	    ad_waitingLine = MAXWAITPEOPLE;
    	    printf("%d rejected %d wait-line %d at %02d:%02d:%02d\n", ad_arrivals, ad_rejected, ad_waitingLine, t/60, t%60,0);
        }
    
        sem_post(&ad_waitingLineMutex);
        //  Simulate the 1 second per virtual minute
        sleep(1);
    }
    
    pthread_exit(NULL);
}

void *ad_explorerCarThread(void *arg) {
    for (int t = 0; t < MAX_TIME; ++t) {
        sem_wait(&ad_carMutex);
        //Explorer Car Logic
        
        sem_post($ad_carMutex);
        sleep(1); //
    //  Implement ford car thread
    //  Make sure to synchronize access to shared resources
    
    pthread_exit(NULL);
}

void *ad_arrivalModuleThread(void *arg) {
    //  Implement arrival module thread
    //  Make sure to synchronize access to shared resources
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    //  Parse command line arguments
    //set MAXPERCAR and CARNUM based on arguments
    
    //Init semaphores
    sem_init(&ad_waitingLineMutex, 0, 1);
    sem_init(&ad_carMutex, 0, 1);
    sem_init(&ad_arrivalMutex, 0, 1);
    
    //create threads
    pthread_t ad_waitingLineThread, ad_explorerCarThreads[MAX_THREADS], arrivalModuleThread;
    
    pthread_create(&ad_waitingLineThread, NULL, ad_waitingLineThread, NULL);
    for(int i = 0; i < CARNUM; ++i) {
        pthread_create(&ad_explorerCarThreads[i], NULL, ad_explorerCarThread, NULL);
    }
    pthread_create(&ad_arrivalModuleThread, NULL, ad_arrivalModuleThread, NULL);
    
    //join the threads
    pthread_join(ad_waitingLineThread, NULL);
    for( int i = 0; i < CARNUM; ++i) {
        pthread_join(ad_explorerCarThreads[i], NULL);
    }
    pthread_join(ad_arrivalModuleThread, NULL);
    
    //ouput statistics
    
    //Destroy semaphores
    sem_destroy(&ad_waitingLineMutex);
    sem_destroy(&ad_carMutex);
    sem_destroy(&ad_arrivalMutex);
    
    return 0;
}

double U_Random(); /*Part of random437.h */
int poissonRandom(); /*Part of random437.h */

int main(int argc, char *argv[])
{
    int CARNUM = 10;       // Default value
    int MAXPERCAR = 4;      // Default value

    // Command line options
    int opt;
    while ((opt = getopt(argc, argv, "N:M:")) != -1)
    {
        switch (opt)
        {
        case 'N':
            CARNUM = atoi(optarg);
            break;
        case 'M':
            MAXPERCAR = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s -N CARNUM -M MAXPERCAR\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Validate command line arguments
    if (CARNUM <= 0 || MAXPERCAR <= 0)
    {
        fprintf(stderr, "Invalid values for CARNUM or MAXPERCAR.\n");
        exit(EXIT_FAILURE);
    }

    printf("N:%d, M: %d\n", CARNUM, MAXPERCAR);

    return 0;
}


/*
  
  Part of random437.h 
  function: called poissonRandom
  Input: is meanArrival
  Output: k..?
  
  
  */
int poissonRandom(int meanArrival)
{
    int k = 0;
    long double p = 1.0;
    long double l = exp(-meanArrival);
    double u = U_Random();
    double F = l;

    while (u >= F)
    {
        k++;
        l *= (double)meanArrival / k;
        F += l;
    }

    return k;
}

double U_Random()
{
    return (double)rand() / RAND_MAX;
}
