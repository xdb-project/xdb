/**
 * @file utils.c
 * @brief General utility functions for the Database system.
 *
 * Provides helper functions for unique ID generation and system logging.
 * These implementations support the core database operations by managing
 * identity and observability.
 */

#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief Character set used for random UUID generation.
 * * Contains alphanumeric characters (a-z, A-Z, 0-9).
 */
static const char CHARSET[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/**
 * @brief Generates a random 16-character UUID string.
 * * Allocates memory for a 16-character string plus a null terminator.
 * The randomness is based on the current state of the global rand() generator.
 *
 * @return char* A pointer to the newly created ID string, or NULL if memory allocation fails.
 * @note The caller is responsible for freeing the memory allocated by this function using free().
 */
char *utils_gen_uuid(void)
{
    int len = 16;
    char *str = malloc(len + 1);

    if (!str) {
        return NULL;
    }

    /* Generate random characters from CHARSET */
    for (int i = 0; i < len; i++) {
        int key = rand() % (int) (sizeof(CHARSET) - 1);
        str[i] = CHARSET[key];
    }

    str[len] = '\0';
    return str;
}

/**
 * @brief Prints a log message to the standard output with a timestamp.
 * * Formats the current system time and outputs a structured log line.
 * Output format: `[HH:MM:SS] [LEVEL] Message`
 *
 * @param[in] level The severity level of the log (e.g., "INFO", "ERROR", "DEBUG").
 * @param[in] msg   The message content to be logged.
 */
void utils_log(const char *level, const char *msg)
{
    time_t now;
    time(&now);
    char buf[20];

    /* Format time as HH:MM:SS */
    struct tm *timeinfo = localtime(&now);
    if (timeinfo) {
        strftime(buf, sizeof(buf), "%H:%M:%S", timeinfo);
        /* Print: [TIME] [LEVEL] Message */
        printf("[%s] [%s] %s\n", buf, level, msg);
    }
}
