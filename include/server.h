/**
 * @file server.h
 * @brief TCP Networking Interface.
 *
 * This module handles the socket lifecycle, including binding, listening,
 * and managing incoming client connections for the database service.
 */

#ifndef SERVER_H
#define SERVER_H

/**
 * @brief Starts the TCP server loop.
 * * This function initializes a socket, binds it to the specified port,
 * and enters a blocking loop to accept incoming client connections.
 * * @param[in] port The port number to bind the server to (e.g., 8080).
 * @note This function blocks indefinitely. To stop the server, an external
 * signal or interrupt handler is typically required.
 */
void server_start(int port);

#endif /* SERVER_H */
