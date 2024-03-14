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
#include <string.h>
#include <memory.h>

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
    ResponseTime **responseTimes;
} Subfield;

typedef struct CallType
{
    char *name;
    int total;
    Subfield **subfields;
} CallType;

int calltype_count = 0;
CallType **call_types = NULL;

/**
 * Create a call type with the provided name and specified subfields.
 */
CallType *init_calltype(char *name, char **subfields)
{
    struct CallType *call_type = malloc(sizeof(struct CallType));
    call_type->subfields = NULL;

    int i = 0;
    while (subfields[i])
    {
        struct Subfield *subfield = malloc(sizeof(struct Subfield));
        struct ResponseTime *dispatch = malloc(sizeof(struct ResponseTime));
        struct ResponseTime *on_scene = malloc(sizeof(struct ResponseTime));
        dispatch->type = DISPATCH;
        on_scene->type = ON_SCENE;

        subfield->name = subfields[i];
        subfield->responseTimes = malloc(2 * sizeof(struct ResponseTime));
        subfield->responseTimes[0] = dispatch;
        subfield->responseTimes[1] = on_scene;

        call_type->subfields = realloc(call_type->subfields, (i + 1) * sizeof(struct Subfield));
        call_type->name = name;
        call_type->total = 0;
        call_type->subfields[i] = subfield;
        i++;
    }
    call_type->subfields[i] = NULL;
    return call_type;
}

void free_calltype(CallType *call)
{
    if (!call)
        return;

    // Guaranteed to contain all fields through `init_calltype()`
    for (int i = 0; call->subfields[i]; i++)
    {
        free(call->subfields[i]->responseTimes[0]);
        free(call->subfields[i]->responseTimes[1]);
        free(call->subfields[i]->responseTimes);
        free(call->subfields[i]);
    }
    free(call->subfields);
    free(call);
}

CallType *find_calltype(char *name)
{
    for (int i = 0; call_types[i]; i++)
    {
        if (strcmp(name, call_types[i]->name) == 0)
        {
            return call_types[i];
        }
    }
    return NULL;
}

/** Get the index of the specified field name */
int get_field_index(char *value)
{
    for (int i = 0; header[i]; i++)
    {
        if (strcmp(value, header[i]) == 0)
            return i;
    }
    return -1;
}

/**
 * Parse the provided timestamp into `time_t`. Only handles two format
 * as provided in the CSV files.
 */
time_t parse_timestamp(char *ts)
{
    struct tm time;
    if (strptime(ts, "%D %R", &time)) // 7/27/20 12:00
    {
        return mktime(&time);
    }
    else if (strptime(ts, "%m/%d/%Y %r", &time)) // 01/06/2016 08:46:50 PM
    {
        return mktime(&time);
    }
    fprintf(stderr, "Could not parse the timestamp: %s\n", ts);
    return -1;
}

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

    CallType *call = init_calltype("TEST_TYPE", subfields);
    printf("Name: %s\n", call->name);
    for (int i = 0; call->subfields[i]; i++)
    {
        printf("Subfield %d: %s\n", i, call->subfields[i]->name);
    }

    // CallType *res = find_calltype("TEST_TYPE");
    free_calltype(call);

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
