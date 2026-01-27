/**
 * @file database.c
 * @brief Secure storage engine implementation for the Database system.
 *
 * Implements a thread-safe, JSON-backed document database with atomic
 * write-to-disk capabilities to ensure data integrity.
 */

#include "../include/database.h"

#include "../include/query.h"
#include "../include/utils.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/** * @brief Global database state variables.
 */
static char g_db_path[256];                              /**< Destination file path on disk. */
static cJSON *root = NULL;                               /**< In-memory representation of the DB. */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; /**< Monitor for thread safety. */

/**
 * @brief Persist database state to disk using an atomic write pattern.
 * * Writes data to a temporary file first and then performs a rename operation.
 * This prevents data corruption in the event of a system crash during the write.
 * * @note This is an internal helper and does not handle its own locking.
 */
static void _save_internal(void)
{
    if (!root)
        return;

    char *str = cJSON_Print(root);
    if (!str)
        return;

    char tmp_path[300];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", g_db_path);

    FILE *fp = fopen(tmp_path, "w");
    if (fp) {
        fprintf(fp, "%s", str);
        fflush(fp);
        fclose(fp);

        /* Atomic swap of temporary file with actual file */
        if (rename(tmp_path, g_db_path) != 0) {
            perror("Failed to replace database file");
        }
    } else {
        perror("Failed to write temporary database file");
    }
    free(str);
}

/**
 * @brief Initializes the database engine and loads existing data.
 * * Sets up the storage path, seeds the random generator for UUIDs,
 * and parses the existing JSON file if present.
 *
 * @param[in] filepath Path to the JSON storage file.
 */
void db_init(const char *filepath)
{
    pthread_mutex_lock(&lock);
    srand((unsigned int) time(NULL));

    strncpy(g_db_path, filepath, sizeof(g_db_path) - 1);

    FILE *fp = fopen(g_db_path, "r");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long len = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (len > 0) {
            char *data = malloc(len + 1);
            if (data) {
                fread(data, 1, len, fp);
                data[len] = '\0';
                root = cJSON_Parse(data);
                free(data);
            }
        }
        fclose(fp);

        char msg[512];
        snprintf(msg, sizeof(msg), "Storage loaded from: %s", g_db_path);
        utils_log("INFO", msg);
    }

    /* Fallback if file is missing or empty */
    if (!root) {
        root = cJSON_CreateObject();
        utils_log("INFO", "Initialized new database instance");
    }
    pthread_mutex_unlock(&lock);
}

/**
 * @brief Shuts down the database engine.
 * * Safely releases the cJSON root object and nullifies the pointer.
 */
void db_cleanup(void)
{
    pthread_mutex_lock(&lock);
    if (root) {
        cJSON_Delete(root);
        root = NULL;
    }
    pthread_mutex_unlock(&lock);
}

/**
 * @brief Removes all collections and stored data.
 * * Resets the root object and immediately triggers an atomic disk save.
 */
void db_drop_all(void)
{
    pthread_mutex_lock(&lock);
    if (root)
        cJSON_Delete(root);

    root = cJSON_CreateObject();
    _save_internal();
    pthread_mutex_unlock(&lock);
}

/**
 * @brief Inserts a document into a collection.
 * * Automatically ensures a unique `_id` is present and persists changes to disk.
 *
 * @param[in] coll_name Target collection name.
 * @param[in] data      JSON object representing the document.
 * @return true on success, false if input data is NULL.
 */
bool db_insert(const char *coll_name, cJSON *data)
{
    if (!data)
        return false;

    pthread_mutex_lock(&lock);

    cJSON *coll = cJSON_GetObjectItem(root, coll_name);
    if (!coll) {
        coll = cJSON_CreateArray();
        cJSON_AddItemToObject(root, coll_name, coll);
    }

    if (!cJSON_HasObjectItem(data, "_id")) {
        char *uuid = utils_gen_uuid();
        cJSON_AddStringToObject(data, "_id", uuid);
        free(uuid);
    }

    cJSON_AddItemToArray(coll, data);
    _save_internal();
    pthread_mutex_unlock(&lock);
    return true;
}

/**
 * @brief Query documents from a collection.
 * * Iterates through the collection and returns duplicates of matching items.
 *
 * @param[in] coll_name Target collection name.
 * @param[in] query     JSON object defining query conditions.
 * @param[in] limit     Maximum number of documents to return.
 * @return cJSON* A new JSON array containing matched documents.
 */
cJSON *db_find(const char *coll_name, cJSON *query, int limit)
{
    pthread_mutex_lock(&lock);
    cJSON *coll = cJSON_GetObjectItem(root, coll_name);
    cJSON *result = cJSON_CreateArray();

    if (coll) {
        int count = 0;
        cJSON *item = NULL;
        cJSON_ArrayForEach(item, coll)
        {
            if (limit > 0 && count >= limit)
                break;
            if (query_match(item, query)) {
                cJSON_AddItemToArray(result, cJSON_Duplicate(item, 1));
                count++;
            }
        }
    }
    pthread_mutex_unlock(&lock);
    return result;
}

/**
 * @brief Delete a document by its unique identifier.
 * * @param[in] coll_name Target collection name.
 * @param[in] id        Document `_id` value.
 * @return true if the document was found and deleted, false otherwise.
 */
bool db_delete(const char *coll_name, const char *id)
{
    pthread_mutex_lock(&lock);
    cJSON *coll = cJSON_GetObjectItem(root, coll_name);
    if (coll) {
        int idx = 0;
        cJSON *item = NULL;
        cJSON_ArrayForEach(item, coll)
        {
            cJSON *itemId = cJSON_GetObjectItem(item, "_id");
            if (cJSON_IsString(itemId) && strcmp(itemId->valuestring, id) == 0) {
                cJSON_DeleteItemFromArray(coll, idx);
                _save_internal();
                pthread_mutex_unlock(&lock);
                return true;
            }
            idx++;
        }
    }
    pthread_mutex_unlock(&lock);
    return false;
}

/**
 * @brief Counts documents in a collection.
 * * @param[in] coll_name Target collection name.
 * @return int Total document count or 0 if collection does not exist.
 */
int db_count(const char *coll_name)
{
    pthread_mutex_lock(&lock);
    cJSON *coll = cJSON_GetObjectItem(root, coll_name);
    int cnt = coll ? cJSON_GetArraySize(coll) : 0;
    pthread_mutex_unlock(&lock);
    return cnt;
}
