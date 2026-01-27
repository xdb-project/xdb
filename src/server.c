/**
 * @file server.c
 * @brief Core TCP Server implementation for the Database system.
 *
 * This module manages the networking layer, handling dual-stack (IPv4/IPv6)
 * socket management, concurrent client requests via POSIX threads,
 * and the JSON-based communication protocol.
 */

#include "../include/server.h"

#include "../include/database.h"
#include "../include/utils.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 8192

/**
 * @brief Structure to pass client context to worker threads.
 */
typedef struct
{
    int sock;                            /**< Client socket file descriptor. */
    struct sockaddr_storage client_addr; /**< Client network address metadata. */
} client_context_t;

/**
 * @brief Extracts and converts a client IP address to a string.
 *
 * @param[in]  addr     The source socket address structure.
 * @param[out] str_buf  The buffer to store the resulting IP string.
 * @param[in]  buf_len  The size of the destination buffer.
 */
void get_client_ip(struct sockaddr_storage *addr, char *str_buf, size_t buf_len)
{
    if (addr->ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *) addr;
        inet_ntop(AF_INET, &s->sin_addr, str_buf, buf_len);
    } else {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *) addr;
        inet_ntop(AF_INET6, &s->sin6_addr, str_buf, buf_len);
    }
}

/**
 * @brief Sends a formatted JSON response back to the client.
 * * Wraps the status and data into a standardized response object and
 * appends a newline as a protocol delimiter.
 *
 * @param[in] sock  Target client socket.
 * @param[in] code  HTTP-style status code (e.g., 200 for OK, 400 for error).
 * @param[in] msg   Descriptive message for the response.
 * @param[in] data  Optional cJSON object containing response payload.
 */
void send_response(int sock, int code, const char *msg, cJSON *data)
{
    cJSON *resp = cJSON_CreateObject();
    cJSON_AddStringToObject(resp, "status", code == 200 ? "ok" : "error");
    cJSON_AddStringToObject(resp, "message", msg);

    if (data) {
        cJSON_AddItemToObject(resp, "data", data);
    }

    char *output = cJSON_PrintUnformatted(resp);
    write(sock, output, strlen(output));
    write(sock, "\n", 1); /* Protocol delimiter */

    free(output);
    cJSON_Delete(resp);
}

/**
 * @brief Thread entry point for handling individual client communication.
 * * Parses incoming JSON commands, routes them to the database engine, and
 * returns responses. Resources are automatically reclaimed upon thread exit.
 *
 * @param[in] arg Pointer to a heap-allocated client_context_t.
 * @return void* Always NULL.
 */
void *handle_client(void *arg)
{
    /* 1. Detach thread to reclaim resources automatically on exit */
    pthread_detach(pthread_self());

    /* 2. Unpack arguments and free context memory immediately */
    client_context_t *ctx = (client_context_t *) arg;
    int sock = ctx->sock;
    struct sockaddr_storage client_addr = ctx->client_addr;
    free(ctx);

    char buffer[BUFFER_SIZE];
    char ip_str[INET6_ADDRSTRLEN];
    int len;

    get_client_ip(&client_addr, ip_str, sizeof(ip_str));
    char log_msg[128];
    sprintf(log_msg, "Client connected from: %s", ip_str);
    utils_log("INFO", log_msg);

    /* 3. Main Communication Loop */
    while ((len = read(sock, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[len] = '\0';

        /* Ignore empty or whitespace-only messages */
        int is_empty = 1;
        for (int i = 0; i < len; i++) {
            if (!isspace((unsigned char) buffer[i])) {
                is_empty = 0;
                break;
            }
        }
        if (is_empty)
            continue;

        cJSON *req = cJSON_Parse(buffer);
        if (!req) {
            char log_buf[256];
            snprintf(log_buf, sizeof(log_buf), "Invalid JSON request from %s", ip_str);
            utils_log("WARN", log_buf);
            send_response(sock, 400, "Invalid JSON", NULL);
            continue;
        }

        cJSON *action = cJSON_GetObjectItem(req, "action");
        cJSON *coll = cJSON_GetObjectItem(req, "collection");

        if (cJSON_IsString(action)) {
            char *act_str = action->valuestring;
            char *coll_str = cJSON_IsString(coll) ? coll->valuestring : "";

            /* Handle session termination */
            if (strcmp(act_str, "exit") == 0) {
                char log_buf[128];
                snprintf(log_buf, sizeof(log_buf), "Session terminated for %s", ip_str);
                utils_log("INFO", log_buf);
                send_response(sock, 200, "Goodbye!", NULL);
                cJSON_Delete(req);
                break;
            }

            /* Command Routing Logic */
            if (strlen(coll_str) == 0) {
                send_response(sock, 400, "Missing 'collection'", NULL);
            } else if (strcmp(act_str, "insert") == 0) {
                cJSON *data = cJSON_DetachItemFromObject(req, "data");
                if (db_insert(coll_str, data)) {
                    cJSON *id_obj = cJSON_GetObjectItem(data, "_id");
                    cJSON *response = cJSON_CreateObject();
                    if (cJSON_IsString(id_obj)) {
                        cJSON_AddStringToObject(response, "_id", id_obj->valuestring);
                    }
                    utils_log("INFO", "Document inserted");
                    send_response(sock, 200, "Inserted", response);
                } else {
                    utils_log("ERROR", "Insert failed");
                    cJSON_Delete(data);
                    send_response(sock, 500, "Failed to insert", NULL);
                }
            } else if (strcmp(act_str, "find") == 0) {
                cJSON *query = cJSON_GetObjectItem(req, "query");
                cJSON *limit_obj = cJSON_GetObjectItem(req, "limit");
                int limit = cJSON_IsNumber(limit_obj) ? limit_obj->valueint : 0;
                cJSON *result = db_find(coll_str, query, limit);
                send_response(sock, 200, "Success", result);
            } else if (strcmp(act_str, "delete") == 0) {
                cJSON *id = cJSON_GetObjectItem(req, "id");
                if (cJSON_IsString(id) && db_delete(coll_str, id->valuestring)) {
                    send_response(sock, 200, "Deleted", NULL);
                } else {
                    send_response(sock, 404, "Not Found", NULL);
                }
            } else if (strcmp(act_str, "count") == 0) {
                cJSON *d = cJSON_CreateObject();
                cJSON_AddNumberToObject(d, "count", db_count(coll_str));
                send_response(sock, 200, "Success", d);
            } else {
                send_response(sock, 400, "Unknown Action", NULL);
            }
        } else {
            send_response(sock, 400, "Missing 'action'", NULL);
        }
        cJSON_Delete(req);
    }

    close(sock);
    return NULL;
}

/**
 * @brief Initializes and starts the TCP server.
 * * Sets up an IPv6 socket with dual-stack support to accept IPv4-mapped
 * connections. The server runs a continuous loop, spawning a worker
 * thread for every accepted connection.
 *
 * @param[in] port The port number to bind the server to.
 */
void server_start(int port)
{
    int server_fd, new_sock;
    struct sockaddr_in6 address;
    socklen_t addr_size;
    int opt = 1;
    int v6only = 0;

    /* Create IPv6 socket */
    if ((server_fd = socket(AF_INET6, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* Enable dual-stack (IPv4 clients can connect) */
    setsockopt(server_fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&address, 0, sizeof(address));
    address.sin6_family = AF_INET6;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons(port);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    char msg[128];
    sprintf(msg, "XDB Server listening on port %d (Dual Stack IPv4/IPv6)", port);
    utils_log("INFO", msg);

    /* Main Accept Loop */
    while (1) {
        client_context_t *ctx = malloc(sizeof(client_context_t));
        if (!ctx)
            continue;

        addr_size = sizeof(ctx->client_addr);
        new_sock = accept(server_fd, (struct sockaddr *) &ctx->client_addr, &addr_size);

        if (new_sock >= 0) {
            ctx->sock = new_sock;
            pthread_t thread_id;
            if (pthread_create(&thread_id, NULL, handle_client, (void *) ctx) != 0) {
                perror("Thread creation failed");
                free(ctx);
                close(new_sock);
            }
        } else {
            free(ctx);
        }
    }
}
