/**
 * @file main.c
 * @brief Application entry point for the Database system.
 *
 * This module coordinates the startup sequence, including signal handling
 * registration, database engine initialization, and the launch of the
 * multithreaded TCP network server.
 */

#include "../include/database.h"
#include "../include/server.h"
#include "../include/utils.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Handles termination signals (e.g., SIGINT).
 * * Intercepts system interrupts to perform a graceful shutdown. This ensures
 * that the database flushes pending data to disk and releases all allocated
 * memory before the process terminates.
 *
 * @param[in] sig Received signal identifier (e.g., SIGINT).
 */
void sig_handler(int sig)
{
    (void) sig; /* Mark as used to prevent compiler warnings */
    printf("\n");
    utils_log("WARN", "System shutdown initiated via signal interrupt.");

    /* Perform graceful cleanup of the database engine */
    db_cleanup();

    exit(EXIT_SUCCESS);
}

/**
 * @brief Main program entry point.
 * * Sets up the execution environment, initializes persistent storage,
 * and binds the network server to the designated port.
 *
 * @return int Exit status code (0 on successful termination).
 */
int main(void)
{
    /* Register signal handler for Ctrl+C and other interrupts */
    signal(SIGINT, sig_handler);

    utils_log("INFO", "Starting XDB Server...");

    /* Initialize the database with the production data file */
    db_init("data/production.json");

    /** * Start the TCP server loop.
     * @note This call is blocking and will run until a signal is received.
     */
    server_start(8080);

    return EXIT_SUCCESS;
}
