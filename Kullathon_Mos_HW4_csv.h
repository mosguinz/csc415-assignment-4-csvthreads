/**************************************************************
 * Class::  CSC-415-03 Spring 2024
 * Name::  Mos Kullathon
 * Student ID::  921425216
 * GitHub-Name::  mosguinz
 * Project::  Assignment 4 – Processing CSV Data with Threads
 *
 * File::  Kullathon_Mos_HW4_csv.h
 *
 * Description::
 *
 **************************************************************/

/**
 * Opens a CSV file, reads the first line (header), and returns a
 * null-terminated array of C string pointers (a vector), each representing a
 * column name. This function should also prepare the file for reading
 * subsequent lines. Return NULL on failure. The caller should NOT free the
 * memory associated with the header.
 */
char **csvopen(char *filename);

/**
 * Reads the next line from the CSV file opened by csvopen, parses it
 * according to CSV formatting rules (including handling quoted fields, escaped
 * quotes, and embedded newlines), and returns a null-terminated array of
 * strings (vector), each representing a cell value in the row.
 */
char **csvnext(void);

/**
 * Returns the header read by csvopen as a null-terminated array of strings.
 * This can be called at any time after csvopen to retrieve the header. The
 * caller should NOT free the memory associated with the header.
 */
char **csvheader(void);

/**
 * Closes the CSV file and frees any memory still associated with managing the
 * file (including the header. Return -1 if an error otherwise return how many
 * data lines had been read
 */
int csvclose(void);
