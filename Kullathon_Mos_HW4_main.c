/**************************************************************
 * Class::  CSC-415-03 Spring 2024
 * Name::  Mos Kullathon
 * Student ID::  921425216
 * GitHub-Name::  mosguinz
 * Project::  Assignment 4 – Processing CSV Data with Threads
 *
 * File::  Kullathon_Mos_HW4_main.c
 *
 * Description::
 *
 **************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "Kullathon_Mos_HW4_csv.h"

typedef struct ResponseTime
{
    enum ResponseType
    {
        DISPATCH,
        ON_SCENE
    } type;
    int under_2_mins;
    int mins_3_5;
    int mins_6_10;
    int over_10_mins;
} ResponseTime;

typedef struct Subfield
{
    char *name;
    ResponseTime *responseTime;
} Subfield;

typedef struct CallType
{
    char *name;
    int total;
    Subfield **subfields;
} CallType;

CallType **call_types = NULL;

/**
 * Create a call type with the provided name and specified subfields.
 */
CallType *init_calltype(char *name, char **subfields)
{
    struct CallType *call_type = malloc(sizeof(struct CallType));
    call_type->subfields = malloc((3 + 1) * sizeof(struct Subfield));

    for (int i = 0; i < 3; i++)
    {
        struct Subfield *subfield = malloc(sizeof(struct Subfield));
        struct ResponseTime *response_time = malloc(sizeof(struct ResponseTime));
        subfield->name = subfields[i];
        subfield->responseTime = response_time;

        call_type->name = name;
        call_type->total = 0;
        call_type->subfields[i] = subfield;
    }
    return call_type;
}

    CallType *next;
} CallType;

main(int argc, char *argv[])
{
    //***TO DO***  Look at arguments, initialize application

    if (argc < 5)
    {
        fprintf(stderr, "Not enough arguments, expected 5, got %d\n", argc);
        return -1;
    }

    char *filename = argv[1];
    int threads = atoi(argv[2]);
    char *subfield_type = argv[3];
    char **subfields = &argv[4];

    for (int i = 0; subfields[i]; i++)
    {
        printf("Subfield %d: %s\n", i, subfields[i]);
    }

    char **fields = csvopen(filename);
    for (int i = 0; fields[i]; i++)
    {
        printf("Field %d: %s\n", i, fields[i]);
    }

    char **row;
    while ((row = csvnext()))
    {
        ;
    }

    csvclose();

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    // Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************

    // *** TO DO ***  start your thread processing
    //                wait for the threads to finish

    // ***TO DO *** Display Data

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    // Clock output
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
    {
        --sec;
        n_sec = n_sec + 1000000000L;
    }

    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);
    //**************************************************************

    // ***TO DO *** cleanup
}
