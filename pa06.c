#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include <pthread.h>

#define MAXWAITPEOPLE 800
#define OUTPUT_FILE_NAME "simulation_output.txt"

// Function declarations
double U_Random();
int poissonRandom(int meanArrival);

// Structure to hold data for each Explorer car
typedef struct
{
    pthread_t thread;
    int passengers;
} ExplorerCar;

// Global variables
int CARNUM = 10;       // Default value
int MAXPERCAR = 4;      // Default value
int totalPeopleArrived = 0;
int totalPeopleRiding = 0;
int totalPeopleRejected = 0;
double totalWaitingTime = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int peopleInLine = 0;

// Function to simulate the waiting area
void *waitingAreaSimulation(void *arg)
{
    int currentTime = 0;

    // Loop through the simulation time
    while (currentTime < 600)
    {
        // Calculate mean arrival based on the specified time intervals
        int meanArrival;
        if (currentTime >= 0 && currentTime < 180)
            meanArrival = 25;
        else if (currentTime >= 180 && currentTime < 420)
            meanArrival = 45;
        else if (currentTime >= 420 && currentTime < 540)
            meanArrival = 35;
        else
            meanArrival = 25;

        // Generate the number of people arriving this minute using Poisson distribution
        int arrivingPeople = poissonRandom(meanArrival);

        // Check if the waiting area is full
        if (peopleInLine + arrivingPeople > MAXWAITPEOPLE)
        {
            int rejected = arrivingPeople - (MAXWAITPEOPLE - peopleInLine);
            arrivingPeople = MAXWAITPEOPLE - peopleInLine;
            totalPeopleRejected += rejected;
        }

        // Update the total number of people arrived
        totalPeopleArrived += arrivingPeople;

        // Synchronize threads
        pthread_mutex_lock(&mutex);

        // Simulate accepting people into the waiting line
        if (arrivingPeople > 0)
        {
            // Assuming a simple scenario where everyone is accepted into the waiting line
            peopleInLine += arrivingPeople;

            // Log the status
            printf("%03d arrive %03d reject %03d wait-line %03d at %02d:%02d:%02d\n",
                   currentTime, arrivingPeople, totalPeopleRejected, peopleInLine,
                   currentTime / 60, currentTime % 60, 0);
        }

        // Calculate waiting time for people in line
        totalWaitingTime += peopleInLine;

        // Synchronize threads
        pthread_mutex_unlock(&mutex);

        // Sleep for one virtual second (you may adjust this based on your requirements)
        sleep(.01);

        // Increment the current time
        currentTime++;
    }

    // Calculate and update the average waiting time
    if (totalPeopleRiding > 0)
        totalWaitingTime /= totalPeopleRiding;

    // Display the summary at the end of the day
    printf("Total People Arrived: %d\n", totalPeopleArrived);
    printf("Total People Riding: %d\n", totalPeopleRiding);
    printf("Total People Rejected: %d\n", totalPeopleRejected);
    printf("Average Waiting Time per Person: %.2f minutes\n", totalWaitingTime);

    return NULL;
}

// Function to simulate the Explorer cars
void *explorerCarSimulation(void *arg)
{
    ExplorerCar *car = (ExplorerCar *)arg;
    int currentTime = 0;

    // Loop through the simulation time
    while (currentTime < 600)
    {
        // Synchronize threads
        pthread_mutex_lock(&mutex);

        // Simulate the ride completion
        if (car->passengers > 0)
        {
            // Log the status
            printf("Explorer %p departs with %d passengers at %02d:%02d:%02d\n",
                   (void *)car->thread, car->passengers, currentTime / 60, currentTime % 60, 0);

            // Update the total number of people riding
            totalPeopleRiding += car->passengers;

            // Reset the number of passengers for the next ride
            car->passengers = 0;
        }

        // Check if there are people in the waiting line
        if (peopleInLine > 0)
        {
            // Pick people from the waiting line up to the maximum per car
            int passengersToPick = (peopleInLine < MAXPERCAR) ? peopleInLine : MAXPERCAR;
            car->passengers = passengersToPick;

            // Update the waiting line count
            peopleInLine -= passengersToPick;

            // Log the status
            printf("Explorer %p arrives with %d passengers at %02d:%02d:%02d\n",
                   (void *)car->thread, car->passengers, currentTime / 60, currentTime % 60, 0);
        }

        // Synchronize threads
        pthread_mutex_unlock(&mutex);

        // Sleep for one virtual second (you may adjust this based on your requirements)
        sleep(.01);

        // Increment the current time
        currentTime++;
    }

    return NULL;
}

// Poisson random number generation
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

// Uniform random number generation
double U_Random()
{
    return (double)rand() / RAND_MAX;
}

int main(int argc, char *argv[])
{
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
    
    // Open the output file for writing (use absolute path)
    FILE *outputFile = fopen(OUTPUT_FILE_NAME, "w");
    if (outputFile == NULL)
    {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    // Redirect standard output to the output file
    if (dup2(fileno(outputFile), STDOUT_FILENO) == -1)
    {
        perror("Error redirecting standard output");
        exit(EXIT_FAILURE);
    }


    printf("N:%d, M: %d\n", CARNUM, MAXPERCAR);

    // Initialize the Explorer cars
    ExplorerCar cars[CARNUM];
    for (int i = 0; i < CARNUM; i++)
    {
        cars[i].passengers = 0;

        // Create a new thread for each Explorer car
        if (pthread_create(&cars[i].thread, NULL, explorerCarSimulation, &cars[i]) != 0)
        {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
    }

    // Create the waiting area thread
    pthread_t waitingAreaThread;
    if (pthread_create(&waitingAreaThread
, NULL, waitingAreaSimulation, NULL) != 0)
    {
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }

    // Join waiting area thread
    if (pthread_join(waitingAreaThread, NULL) != 0)
    {
        perror("Error joining thread");
        exit(EXIT_FAILURE);
    }

    // Join Explorer car threads
    for (int i = 0; i < CARNUM; i++)
    {
        if (pthread_join(cars[i].thread, NULL) != 0)
        {
            perror("Error joining thread");
            exit(EXIT_FAILURE);
        }
    }
    
    // Close the original file descriptor
    fclose(outputFile);

    // Generate graphs using Python script
    int sysCallResult = system("python3 plotting_script.py");
    if (sysCallResult == -1)
    {
        perror("Error calling Python script");
        exit(EXIT_FAILURE);
    }
    return 0;
}
