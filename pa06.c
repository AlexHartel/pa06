#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>

#define MAXWAITPEOPLE 800


double U_Random();
int poissonRandom();

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
