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

typedef enum ResponseType
{
    DISPATCH,
    ON_SCENE
} ResponseType;

typedef struct ResponseTime
{
    ResponseType type;
    int under_2_mins;
    int mins_3_5;
    int mins_6_10;
    int over_10_mins;
} ResponseTime;

typedef struct Subfield
{
    char *name;
    ResponseTime *responseTimes[2];
} Subfield;

typedef struct CallType
{
    char *name;
    int total;
    Subfield **subfields;
} CallType;

int calltype_count = 0;
CallType **call_types = NULL;
char **header = NULL;

/**
 * Create a call type with the provided name and specified subfields.
 */
CallType *init_calltype(char *name, char **subfields)
{
    struct CallType *call_type = malloc(sizeof(struct CallType));
    call_type->name = strdup(name);
    call_type->total = 0;
    call_type->subfields = NULL;

    int i = 0;
    while (subfields[i])
    {
        struct Subfield *subfield = malloc(sizeof(struct Subfield));
        struct ResponseTime *dispatch = malloc(sizeof(struct ResponseTime));
        struct ResponseTime *on_scene = malloc(sizeof(struct ResponseTime));
        dispatch->type = DISPATCH;
        on_scene->type = ON_SCENE;

        subfield->name = strdup(subfields[i]);
        subfield->responseTimes[DISPATCH] = dispatch;
        subfield->responseTimes[ON_SCENE] = on_scene;

        call_type->subfields = realloc(call_type->subfields, (i + 1) * sizeof(struct Subfield));
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
        free(call->subfields[i]->responseTimes[DISPATCH]);
        free(call->subfields[i]->responseTimes[ON_SCENE]);
        free(call->subfields[i]->name);
        free(call->subfields[i]);
    }
    free(call->name);
    free(call->subfields);
    free(call);
}

CallType *find_calltype(char *name)
{
    if (!call_types)
        return NULL;
    for (int i = 0; call_types[i]; i++)
    {
        if (strcmp(name, call_types[i]->name) == 0)
        {
            return call_types[i];
        }
    }
    return NULL;
}

void update_calltype(CallType *call, char *subfield, ResponseType type, double time_delta)
{
    call->total++;
    for (int i = 0; call->subfields[i]; i++)
    {
        if (strcmp(call->subfields[i]->name, subfield) == 0)
        {
            ResponseTime *resp_time = call->subfields[i]->responseTimes[type];
            if (time_delta <= 120)
                resp_time->under_2_mins++;
            else if (time_delta > 120 && time_delta <= 300)
                resp_time->mins_3_5++;
            else if (time_delta > 300 && time_delta <= 600)
                resp_time->mins_6_10++;
            else
                resp_time->over_10_mins++;
        }
    }
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

/** Helper to free rows read from CSV. */
void free_row(char **row)
{
    for (int i = 0; row[i]; i++)
        free(row[i]);
    free(row);
}

/**
 * Parse the provided timestamp into `time_t`. Only handles two format
 * as provided in the CSV files.
 */
time_t parse_timestamp(char *ts)
{
    struct tm time = {0};
    if (strptime(ts, "%m/%d/%y %H:%M", &time)) // 7/27/20 12:00
    {
        time.tm_sec = 0;
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

    header = csvopen(filename);

    // Field indicies
    int call_type_final_desc = get_field_index("call_type_final_desc");
    int call_type_original_desc = get_field_index("call_type_original_desc");
    int analysis_neighborhood = get_field_index("analysis_neighborhood");
    int police_district = get_field_index("police_district");
    int received_datetime = get_field_index("received_datetime");
    int dispatch_datetime = get_field_index("dispatch_datetime");
    int enroute_datetime = get_field_index("enroute_datetime");
    int onscene_datetime = get_field_index("onscene_datetime");

    int subfield_col = strcmp(subfield_type, "police_district")
                           ? analysis_neighborhood
                           : police_district;

    printf("%d %d %d %d %d %d %d %d\n", call_type_final_desc,
           call_type_original_desc,
           analysis_neighborhood,
           police_district,
           received_datetime,
           dispatch_datetime,
           enroute_datetime,
           onscene_datetime);

    char **row;
    int i = 1; // for debug
    while (row = csvnext())
    {
        i++;
        char *call_type = row[call_type_final_desc][0]
                              ? row[call_type_final_desc]
                              : row[call_type_original_desc];
        char *subfield = row[subfield_col];
        char *received_ts = row[received_datetime];
        char *dispatch_ts = row[dispatch_datetime];
        char *enroute_ts = row[enroute_datetime];
        char *onscene_ts = row[onscene_datetime];

        if (!call_type[0] ||
            !received_ts[0] ||
            !dispatch_ts[0] ||
            !enroute_ts[0] ||
            !onscene_ts[0])
        {
            continue;
        }

        // Parse time

        double dispatch_delta = difftime(parse_timestamp(dispatch_ts), parse_timestamp(received_ts));
        double onscene_delta = difftime(parse_timestamp(onscene_ts), parse_timestamp(enroute_ts));

        CallType *call = find_calltype(call_type);
        if (!call)
        {
            call = init_calltype(call_type, subfields);
            call_types = realloc(call_types, (calltype_count + 1) * sizeof(struct CallType *));
            call_types[calltype_count] = call;
            calltype_count++;
        }
        update_calltype(call, subfield, DISPATCH, dispatch_delta);
        update_calltype(call, subfield, ON_SCENE, onscene_delta);

        printf("Row %d | Call count: %d\n", i, calltype_count);
        printf("Type: %s | Dispatch: %s\n", call_type, dispatch_ts);

        for (int i = 0; row[i]; i++)
        {
            free(row[i]);
        }
        free(row);
        if (i > 56)
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
