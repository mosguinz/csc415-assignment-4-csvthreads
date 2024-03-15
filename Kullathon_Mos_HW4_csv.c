/**************************************************************
 * Class::  CSC-415-03 Spring 2024
 * Name::  Mos Kullathon
 * Student ID::  921425216
 * GitHub-Name::  mosguinz
 * Project::  Assignment 4 – Processing CSV Data with Threads
 *
 * File::  Kullathon_Mos_HW4_csv.c
 *
 * Description::
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

#include "Kullathon_Mos_HW4_csv.h"

static FILE *csv_file = NULL;
static char **header = NULL;
static char *current_line = NULL;
static int lines_read = -1;

/**
 * Check if the provided line is a valid CSV row.
 * A row is complete when all opened quotes are closed.
 */
static bool is_valid_row(char *line)
{
    int quotes = 0;
    for (int i = 0; i < strlen(line); i++)
    {
        if (line[i] == '"')
        {
            quotes++;
        }
    }
    return (quotes % 2) == 0;
}

/**
 * Get the next valid row in the CSV file. The string returned by
 * this function is guaranteed to be valid.
 */
static char *read_row(void)
{
    char *line = malloc(1);
    line[0] = '\0';
    char *fragment = NULL;

    size_t n = 0;
    size_t row_size = 0;
    ssize_t length;
    while ((length = getline(&fragment, &n, csv_file)) != -1)
    {
        row_size += length;
        line = realloc(line, (row_size + 1) * sizeof(char *));
        strcat(line, fragment);
        free(fragment);
        if (is_valid_row(line))
        {
            return line;
        }
    }

    free(line);
    free(fragment);
    return NULL;
}

/**
 * Parse a string that is a valid CSV row into a NULL-terminated
 * array of string representing each columns.
 */
static char **parse_csv_line(char *line)
{
    // Allocate memory for an initial number of fields
    char **fields = NULL;
    int field_count = 0;
    bool in_quotes = false;
    char *field = (char *)malloc(strlen(line) + 1);
    if (!field)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    size_t field_size = 0;

    for (size_t i = 0; line[i] != '\0'; i++)
    {
        fields = realloc(fields, (field_count + 1) * sizeof(char *));

        if (line[i] == '"')
        {
            // Toggle in_quotes
            in_quotes = !in_quotes;

            // Handle consecutive quotes inside a quoted field ("")
            if (line[i + 1] == '"')
            {
                in_quotes = !in_quotes;

                if (in_quotes)
                {
                    field[field_size++] = '"';
                }
                i++; // Skip the next quote
                continue;
            }
        }
        else if (line[i] == ',' && !in_quotes)
        {
            // End of field
            field[field_size] = '\0';
            fields[field_count++] = strdup(field);
            field_size = 0;
            continue;
        }
        else if (line[i] == '\n' && !in_quotes)
        {
            // End of line
            field[field_size] = '\0';
            fields[field_count++] = strdup(field);
            field_size = 0;
            break;
        }
        else
        {
            // Add character to field
            field[field_size++] = line[i];
        }
    }

    free(field);
    field = NULL;

    // Reallocate memory to fit actual number of fields
    fields = realloc(fields, (field_count + 1) * sizeof(char *));
    fields[field_count] = NULL;

    lines_read++;
    return fields;
}

char **csvopen(char *filename)
{
    csv_file = fopen(filename, "r");
    if (!csv_file)
    {
        perror("Failed to open file");
        return NULL;
    }

    // Read header
    current_line = read_row();
    if (!current_line)
    {
        fprintf(stderr, "Error reading line from CSV\n");
        fclose(csv_file);
        return NULL;
    }

    // Parse header
    header = parse_csv_line(current_line);
    return header;
}

char **csvnext(void)
{
    if (!csv_file || feof(csv_file))
        return NULL;

    // Read next line
    free(current_line);
    current_line = read_row();
    if (!current_line)
        return NULL;

    return parse_csv_line(current_line);
}

char **csvheader(void)
{
    return header;
}

int csvclose(void)
{
    // Close the file.
    if (csv_file)
    {
        fclose(csv_file);
        csv_file = NULL;
    }

    // Free everything.
    if (header)
    {
        for (int i = 0; header[i] != NULL; i++)
        {
            free(header[i]);
        }
        free(header);
        header = NULL;
    }
    if (current_line)
    {
        free(current_line);
        current_line = NULL;
    }

    return lines_read;
}
