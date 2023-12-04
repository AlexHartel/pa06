#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include <pthread.h>

#define MAXWAITPEOPLE 800
#define OUTPUT_FILE_NAME "simulation_output.txt"

int SIMULATION_DURATION = 600;

// Function declarations
void redirectOutputToFile(int fileDescriptor);
double U_Random();
int poissonRandom(int meanArrival);

// Structure to hold data for each Explorer car
typedef struct
{
    pthread_t thread;
    int passengers;
    FILE *outputFile;
    int N;  // Number of Explorer cars
    int M;  // Maximum passengers per car
    int index;  // Index of the Explorer car
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
ExplorerCar cars[N];


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

    // Open a separate file for each Explorer car
    char outputFileName[50];
    sprintf(outputFileName, "Simulation_Output_N%dM%d_Explorer%d.txt", car->N, car->M, car->index);
    car->outputFile = fopen(outputFileName, "w");
    if (car->outputFile == NULL)
    {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    // Redirect output to the specific file for this Explorer car
    redirectOutputToFile(fileno(car->outputFile));

    // Loop through the simulation time
    while (currentTime < 600)
    {
        // ... (rest of the code remains unchanged)

        // Increment the current time
        currentTime++;
    }

    // Close the output file for this Explorer car
    fclose(car->outputFile);

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

// Function to run the simulation for a given N and M
void runSimulation(ExplorerCar cars[], int N, int M, int simulationNumber)
{
    // Reset global variables
    totalPeopleArrived = 0;
    totalPeopleRiding = 0;
    totalPeopleRejected = 0;
    totalWaitingTime = 0;

    // Initialize the Explorer cars and create threads
    for (int i = 0; i < N; i++)
    {
        cars[i].passengers = 0;
        cars[i].N = N;
        cars[i].M = M;
        cars[i].index = i;

        // Open a separate file for each Explorer car
        char outputFileName[50];
        sprintf(outputFileName, "Simulation_Output_N%dM%d_Explorer%d.txt", N, M, i);
        cars[i].outputFile = fopen(outputFileName, "w");
        if (cars[i].outputFile == NULL)
        {
            perror("Error opening output file");
            exit(EXIT_FAILURE);
        }

        // Create a new thread for each Explorer car
        if (pthread_create(&cars[i].thread, NULL, explorerCarSimulation, &cars[i]) != 0)
        {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
    }

    // Join Explorer car threads and close the output files
    for (int i = 0; i < N; i++)
    {
        if (pthread_join(cars[i].thread, NULL) != 0)
        {
            perror("Error joining thread");
            exit(EXIT_FAILURE);
        }

        // Close the output file for each Explorer car
        fclose(cars[i].outputFile);
    }
}


int main(int argc, char *argv[])
{
    // Command line options
    int opt;
    int N_values[10]; // Assuming a maximum of 10 N values
    int M_values[10]; // Assuming a maximum of 10 M values
    int N_count = 0;
    int M_count = 0;

    // Parse command line options
    while ((opt = getopt(argc, argv, "N:M:")) != -1)
    {
        switch (opt)
        {
        case 'N':
            N_values[N_count++] = atoi(optarg);
            break;
        case 'M':
            M_values[M_count++] = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s -N N1,N2,... -M M1,M2,...\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Validate command line arguments
    if (N_count == 0 || M_count == 0)
    {
        fprintf(stderr, "Invalid values for N or M.\n");
        exit(EXIT_FAILURE);
    }

    // Run the simulation for each combination of N and M
    for (int i = 0; i < N_count; i++)
    {
        for (int j = 0; j < M_count; j++)
        {
            // Add a unique identifier for each simulation
            int simulationNumber = i * M_count + j;

            // Call the runSimulation function with the correct number of arguments
            runSimulation(cars, N_values[i], M_values[j], simulationNumber);
            printf("\n"); // Add a separator between different simulations
        }
    }

    return 0;
}
