/**
 * @file utils.h
 * @brief General utility functions for ID generation and logging.
 *
 * This header provides auxiliary functions used across the database engine,
 * including unique identifier generation and standardized logging.
 */

#ifndef UTILS_H
#define UTILS_H

/**
 * @brief Generates a random 16-character alphanumeric UUID.
 * * This function creates a unique string identifier typically used for
 * document `_id` fields.
 * * @return char* Pointer to a null-terminated string containing the UUID.
 * @note The caller is responsible for freeing the returned string using free().
 */
char *utils_gen_uuid(void);

/**
 * @brief Logs a message to the console with a timestamp.
 * * Formats and outputs system messages to standard streams for
 * monitoring and debugging.
 *
 * @param[in] level The severity level (e.g., "INFO", "ERROR", "DEBUG").
 * @param[in] msg   The message content to be logged.
 */
void utils_log(const char *level, const char *msg);

#endif /* UTILS_H */
