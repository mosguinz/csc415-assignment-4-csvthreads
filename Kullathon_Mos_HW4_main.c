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
#include "Kullathon_Mos_HW4_main.h"

int calltype_count = 0;
CallType **call_types = NULL;
char **header = NULL;
int subfield_count = 0;
char *subfields[4];
char *subfield_type;

ResponseTime *init_response_time(ResponseType type)
{
    struct ResponseTime *rt = malloc(sizeof(struct ResponseTime));
    rt->type = type;
    rt->under_2_mins = 0;
    rt->mins_3_5 = 0;
    rt->mins_6_10 = 0;
    rt->over_10_mins = 0;
    return rt;
}

Subfield *init_subfield(char *name)
{
    struct Subfield *subfield = malloc(sizeof(struct Subfield));
    struct ResponseTime *dispatch = init_response_time(DISPATCH);
    struct ResponseTime *on_scene = init_response_time(ON_SCENE);

    subfield->name = strdup(name);
    subfield->responseTimes[DISPATCH] = dispatch;
    subfield->responseTimes[ON_SCENE] = on_scene;
    return subfield;
}

/**
 * Create a call type with the provided name and specified subfields.
 */
CallType *init_calltype(char *name, char **subfields)
{
    struct CallType *call = malloc(sizeof(struct CallType));
    call->name = strdup(name);
    call->total_count = 0;
    call->subfields = NULL;
    call->call_total = init_subfield("Total");

    int i = 0;
    while (subfields[i])
    {
        call->subfields = realloc(call->subfields, (i + 1) * sizeof(struct Subfield));
        call->subfields[i] = init_subfield(subfields[i]);
        i++;
    }
    call->subfields[i] = NULL;
    return call;
}

void free_subfield(Subfield *subfield)
{
    if (!subfield)
        return;
    free(subfield->responseTimes[DISPATCH]);
    free(subfield->responseTimes[ON_SCENE]);
    free(subfield->name);
    free(subfield);
}

void free_calltype(CallType *call)
{
    if (!call)
        return;

    // Guaranteed to contain all fields through `init_calltype()`
    for (int i = 0; call->subfields[i]; i++)
    {
        free_subfield(call->subfields[i]);
    }
    free_subfield(call->call_total);
    free(call->name);
    free(call->subfields);
    free(call);
}

CallType *find_calltype(char *name)
{
    if (!call_types)
        return NULL;
    for (int i = 0; i < calltype_count; i++)
    {
        if (strcmp(name, call_types[i]->name) == 0)
        {
            return call_types[i];
        }
    }
    return NULL;
}

void update_response_time(ResponseTime *resp_time, double time_delta)
{
    if (time_delta <= 120)
        resp_time->under_2_mins++;
    else if (time_delta > 120 && time_delta <= 300)
        resp_time->mins_3_5++;
    else if (time_delta > 300 && time_delta <= 600)
        resp_time->mins_6_10++;
    else
        resp_time->over_10_mins++;
}

void update_calltype(CallType *call, char *subfield, ResponseType type, double time_delta)
{
    update_response_time(call->call_total->responseTimes[type], time_delta);

    for (int i = 0; call->subfields[i]; i++)
    {
        if (strcmp(call->subfields[i]->name, subfield) == 0)
        {
            ResponseTime *resp_time = call->subfields[i]->responseTimes[type];
            update_response_time(resp_time, time_delta);
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

void print_header_field(char *text, int width)
{
    int l_pad = (width - strlen(text)) / 2;
    int r_pad = strlen(text) % 2 == 0 ? l_pad + 1 : l_pad;
    printf("%*s%s%*s|", l_pad, "", text, r_pad, "");
}

/** An implementation of insertion sort to sort the calls. */
void sort_calls()
{
    int i, j;
    CallType *call;
    for (i = 1; i < calltype_count; i++)
    {
        call = call_types[i];
        j = i - 1;
        while (j >= 0 && strcmp(call_types[j]->name, call->name) > 0)
        {
            call_types[j + 1] = call_types[j];
            j = j - 1;
        }
        call_types[j + 1] = call;
    }
}

void display_calltypes()
{
    // Keep track of field sizes to pad columns.
    int max_name_len = 25;
    int max_digits = 0;
    int max_col_value = 0;

    // Calculate field sizes, etc.
    for (int i = 0; i < calltype_count; i++)
    {
        CallType *call = call_types[i];
        max_col_value = call->total_count > max_col_value ? call->total_count : max_col_value;

        for (int j = 0; call->subfields[j]; j++)
        {
            size_t name_len = strlen(call->name);
            max_name_len = name_len > max_name_len ? name_len : max_name_len;
        }
    }

    // Find max number of digits in the total column.
    while (max_col_value != 0)
    {
        max_col_value /= 10;
        max_digits++;
    }
    max_digits = max_digits > 5 ? max_digits : 5;

    // Print headers
    int subfield_width = (max_digits + 2) * 4 + 3;

    // Subfield names
    printf("%*s|", max_name_len + max_digits + 4, "");
    for (int i = -1; subfields[i]; i++)
    {
        char *name = i == -1 ? "Total" : subfields[i];
        print_header_field(name, subfield_width);
        print_header_field(name, subfield_width);
    }
    printf("\n");

    // Dispatch times
    printf("%*s|", max_name_len + max_digits + 4, "");
    for (int i = -1; subfields[i]; i++)
    {
        print_header_field("Dispatch Time", subfield_width);
        print_header_field("On Scene Time", subfield_width);
    }
    printf("\n");

    // Time buckets
    int number_width = max_digits + 2;
    printf("%-*s |", max_name_len, "Call Type");
    print_header_field("Total", max_digits + 2);
    for (int i = 0; i < (subfield_count * 2) + 2; i++)
    {
        print_header_field("<2", number_width);
        print_header_field("3-5", number_width);
        print_header_field("6-10", number_width);
        print_header_field(">10", number_width);
    }
    printf("\n");

    // Print the line below the header.
    for (int i = 0; i < max_name_len + max_digits + 4; i++)
        printf("-");
    printf("+");

    for (int i = 0; i < (subfield_count * 2) + 2; i++)
    {
        for (int j = 0; j < subfield_width; j++)
            printf("-");
        printf("+");
    }

    printf("\n");

    // Print each rows.
    sort_calls();
    for (int i = 0; i < calltype_count; i++)
    {
        CallType *call = call_types[i];
        Subfield *total = call->call_total;

        // Print call type and total.
        printf("%-*s | %*d | ", max_name_len, call->name, max_digits, call->total_count);

        // Print count for each subfields.
        for (int j = -1; call->subfields[j]; j++)
        {
            Subfield *subfield = j == -1 ? total : call->subfields[j];

            ResponseTime *dispatch = subfield->responseTimes[DISPATCH];
            ResponseTime *on_scene = subfield->responseTimes[ON_SCENE];
            printf("%*d | %*d | %*d | %*d | %*d | %*d | %*d | %*d | ",
                   max_digits,
                   dispatch->under_2_mins,
                   max_digits,
                   dispatch->mins_3_5,
                   max_digits,
                   dispatch->mins_6_10,
                   max_digits,
                   dispatch->over_10_mins,
                   max_digits,
                   on_scene->under_2_mins,
                   max_digits,
                   on_scene->mins_3_5,
                   max_digits,
                   on_scene->mins_6_10,
                   max_digits,
                   on_scene->over_10_mins);
        }
        printf("\n");
    }
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

void *process(void *args)
{
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

    char **row;
    while (row = csvnext())
    {
        char *call_type = row[call_type_final_desc][0]
                              ? row[call_type_final_desc]
                              : row[call_type_original_desc];
        char *subfield = row[subfield_col];
        char *received_ts = row[received_datetime];
        char *dispatch_ts = row[dispatch_datetime];
        char *enroute_ts = row[enroute_datetime];
        char *onscene_ts = row[onscene_datetime];

        if (!call_type[0] || !received_ts[0] || !dispatch_ts[0] ||
            !enroute_ts[0] || !onscene_ts[0] || !subfield[0])
        {
            free_row(row);
            continue;
        }

        // Parse time
        double dispatch_delta = difftime(parse_timestamp(dispatch_ts),
                                         parse_timestamp(received_ts));
        double onscene_delta = difftime(parse_timestamp(onscene_ts),
                                        parse_timestamp(enroute_ts));

        CallType *call = find_calltype(call_type);
        if (!call)
        {
            call = init_calltype(call_type, subfields);
            call_types = realloc(call_types, (calltype_count + 1) * sizeof(struct CallType *));
            call_types[calltype_count] = call;
            calltype_count++;
        }
        call->total_count++;
        update_calltype(call, subfield, DISPATCH, dispatch_delta);
        update_calltype(call, subfield, ON_SCENE, onscene_delta);

        free_row(row);
    }

    call_types = realloc(call_types, (calltype_count + 1) * sizeof(struct CallType *));
    call_types[calltype_count] = NULL;
    printf("Setting call_types[%d] as NULL\n", calltype_count);
}

int main(int argc, char *argv[])
{
    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    // Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************

    //***TO DO***  Look at arguments, initialize application

    if (argc < 5)
    {
        fprintf(stderr, "Not enough arguments, expected 5, got %d\n", argc);
        return -1;
    }

    char *filename = argv[1];
    int thread_count = atoi(argv[2]);
    subfield_type = argv[3];
    for (int i = 4; argv[i]; i++)
    {
        subfields[subfield_count] = argv[i];
        subfield_count++;
    }

    header = csvopen(filename);

    // *** TO DO ***  start your thread processing
    //                wait for the threads to finish

    // ***TO DO *** Display Data

    pthread_t *threads = (pthread_t *)malloc(thread_count * sizeof(pthread_t));
    int i = 0;
    while (i < thread_count)
    {
        pthread_create(&threads[i], NULL, process, NULL);
        i++;
    }
    i = 0;
    while (i < thread_count)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    process(NULL);
    display_calltypes();

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

    for (int i = 0; i < calltype_count; i++)
    {
        CallType *call = call_types[i];
        free_calltype(call);
    }

    free(call_types);
    csvclose();
}
