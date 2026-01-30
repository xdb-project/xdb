/**
 * @file database.h
 * @brief Core Database Interface Engine.
 *
 * This header provides the API for a lightweight JSON-based document store.
 * It handles basic CRUD operations, persistence to disk, and snapshot management.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include "../third_party/cJSON/cJSON.h"

#include <stdbool.h>

/**
 * @brief Initializes the database engine.
 *
 * Loads existing data from the disk into memory or creates a new storage file
 * if one does not exist. This must be called before any other DB operations.
 *
 * @param[in] filepath Path to the JSON storage file (e.g., "data/production.json").
 */
void db_init(const char *filepath);

/**
 * @brief Gracefully shuts down the database.
 *
 * Flushes all in-memory data to the disk and releases all allocated
 * internal resources to prevent memory leaks.
 */
void db_cleanup(void);

/**
 * @brief Forces an immediate snapshot of the database.
 *
 * Manually triggers a backup of the current production data to a timestamped
 * file in the storage directory.
 */
void db_force_snapshot(void);

/**
 * @brief Removes all collections and stored data.
 *
 * Clears all data from both memory and the physical storage file.
 * @warning This operation is irreversible and intended for reset/testing only.
 */
void db_drop_all(void);

/**
 * @brief Inserts a new document into a collection.
 *
 * Automatically generates a unique `_id` field for the document before insertion.
 *
 * @param[in] collection The name of the target collection.
 * @param[in] data       A cJSON object representing the document data.
 * @return true if the insertion was successful, false otherwise.
 */
bool db_insert(const char *collection, cJSON *data);

/**
 * @brief Queries documents from a collection.
 *
 * Performs basic key-value matching based on the provided query object.
 *
 * @param[in] collection The name of the target collection.
 * @param[in] query      cJSON object defining match conditions (NULL to match all).
 * @param[in] limit      Maximum number of documents to return (0 for no limit).
 * @return A cJSON array containing matching documents, or NULL on failure.
 * @note The caller is responsible for freeing the returned cJSON object using cJSON_Delete().
 */
cJSON *db_find(const char *collection, cJSON *query, int limit);

/**
 * @brief Performs a selective update on an existing document.
 *
 * Merges new fields into a document identified by its unique `_id`.
 * @note The `_id` field is immutable and cannot be changed through this operation.
 * Fields not present in the input data will remain unchanged in the document.
 *
 * @param[in] collection The name of the target collection.
 * @param[in] id         The unique `_id` string of the document to update.
 * @param[in] data       A cJSON object containing the fields to update or add.
 * @return true if the document was found and updated, false otherwise.
 */
bool db_update(const char *collection, const char *id, cJSON *data);

/**
 * @brief Updates an existing document or inserts it if not found.
 *
 * If a document with the given `id` exists, it performs a selective update.
 * If it does not exist, it inserts the data as a new document.
 *
 * @param[in] collection The name of the target collection.
 * @param[in] id         The unique `_id` string of the document (can be NULL for forced insert).
 * @param[in] data       A cJSON object representing the document data.
 * @return true on successful update or insertion, false otherwise.
 */
bool db_upsert(const char *collection, const char *id, cJSON *data);

/**
 * @brief Deletes a document by its unique identifier.
 *
 * @param[in] collection The name of the target collection.
 * @param[in] id         The unique `_id` string of the document to delete.
 * @return true if the document was deleted, false if the ID was not found.
 */
bool db_delete(const char *collection, const char *id);

/**
 * @brief Counts the number of documents in a collection.
 *
 * @param[in] collection The name of the target collection.
 * @return The total number of documents in the collection.
 */
int db_count(const char *collection);

#endif /* DATABASE_H */
