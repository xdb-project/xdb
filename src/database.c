/**
 * @file database.c
 * @brief Secure storage engine implementation for the Database system.
 *
 * Implements a thread-safe, JSON-backed document database with atomic
 * write-to-disk capabilities, automatic snapshotting, and fast indexing.
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
static cJSON *g_index = NULL;                            /**< Global index for O(1) ID lookups. */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; /**< Monitor for thread safety. */
static int g_op_counter = 0;                             /**< Counter to trigger snapshots. */
static bool g_test_mode = false; /**< Flag to suppress snapshots during tests. */

/**
 * @brief Rebuilds the in-memory index for fast lookups.
 * @note Must be called within a locked mutex context.
 */
static void _rebuild_index(void)
{
    if (g_index)
        cJSON_Delete(g_index);
    g_index = cJSON_CreateObject();

    /* Safety Check */
    if (!root)
        return;

    cJSON *coll = root->child;
    while (coll) {
        cJSON *doc = coll->child;
        while (doc) {
            cJSON *id = cJSON_GetObjectItem(doc, "_id");
            if (id && id->valuestring) {
                /* Deep copy to ensure index stability */
                cJSON_AddItemToObject(g_index, id->valuestring, cJSON_Duplicate(doc, 1));
            }
            doc = doc->next;
        }
        coll = coll->next;
    }
}

/**
 * @brief Creates a physical copy of the current database file with a timestamp.
 * * Provides a "restore point" by copying the production file to a new
 * timestamped file in the data directory.
 * * @note This is an internal helper called by _save_internal and db_force_snapshot.
 */
static void _create_snapshot(void)
{
    char backup_path[512];
    time_t now = time(NULL);
    const struct tm *t = localtime(&now);

    /* Generate filename format: data/backup_YYYYMMDD_HHMM.json */
    snprintf(backup_path, sizeof(backup_path), "data/backup_%04d%02d%02d_%02d%02d.json",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);

    FILE *src = fopen(g_db_path, "rb");
    FILE *dst = fopen(backup_path, "wb");

    if (src && dst) {
        char buf[8192];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
            fwrite(buf, 1, n, dst);
        }

        char log_msg[600];
        snprintf(log_msg, sizeof(log_msg), "Snapshot created: %s", backup_path);
        utils_log("INFO", log_msg);
    }

    if (src)
        fclose(src);
    if (dst)
        fclose(dst);
}

/**
 * @brief Persist database state to disk using an atomic write pattern.
 *
 * Writes data to a temporary file first and then performs a rename operation.
 * Also triggers a snapshot every 5 successful write operations.
 *
 * @note This is an internal helper and does not handle its own locking.
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
        if (rename(tmp_path, g_db_path) == 0) {
            /* Trigger snapshotting logic every 5 operations unless in test mode */
            if (!g_test_mode) {
                g_op_counter++;
                if (g_op_counter >= 5) {
                    _create_snapshot();
                    g_op_counter = 0;
                }
            }
        } else {
            perror("Failed to replace database file");
        }
    } else {
        perror("Failed to write temporary database file");
    }
    free(str);
}

/**
 * @brief Initializes the database engine and loads existing data.
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
    }

    if (!root) {
        root = cJSON_CreateObject();
        utils_log("INFO", "Initialized new database instance");
    }

    /* Build index for the first time */
    _rebuild_index();

    char msg[512];
    snprintf(msg, sizeof(msg), "Storage loaded and indexed from: %s", g_db_path);
    utils_log("INFO", msg);

    pthread_mutex_unlock(&lock);
}

/**
 * @brief Shuts down the database engine.
 */
void db_cleanup(void)
{
    pthread_mutex_lock(&lock);
    if (root) {
        cJSON_Delete(root);
        root = NULL;
    }
    if (g_index) {
        cJSON_Delete(g_index);
        g_index = NULL;
    }
    pthread_mutex_unlock(&lock);
}

/**
 * @brief Enables or disables testing mode.
 *
 * @param[in] enable True to suppress snapshots, false for normal operation.
 */
void db_set_test_mode(bool enable)
{
    pthread_mutex_lock(&lock);
    g_test_mode = enable;
    pthread_mutex_unlock(&lock);
}

/**
 * @brief Forces an immediate snapshot of the current database state.
 * * Manually triggers the creation of a restore point.
 */
void db_force_snapshot(void)
{
    pthread_mutex_lock(&lock);
    _create_snapshot();
    pthread_mutex_unlock(&lock);
}

/**
 * @brief Removes all collections and stored data.
 */
void db_drop_all(void)
{
    pthread_mutex_lock(&lock);
    if (root)
        cJSON_Delete(root);
    if (g_index)
        cJSON_Delete(g_index);

    root = cJSON_CreateObject();
    g_index = cJSON_CreateObject();
    _save_internal();
    pthread_mutex_unlock(&lock);
}

/**
 * @brief Inserts a document into a collection.
 * * @note CRITICAL STABILITY FIX: This function now creates a DEEP COPY
 * of the input data before storing it. This prevents double-free corruption
 * when the server network layer frees the request payload.
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

    /* Ensure ID exists */
    if (!cJSON_HasObjectItem(data, "_id")) {
        char *uuid = utils_gen_uuid();
        cJSON_AddStringToObject(data, "_id", uuid);
        free(uuid);
    }

    /* Update index with a deep copy */
    cJSON *id = cJSON_GetObjectItem(data, "_id");
    if (id && id->valuestring) {
        cJSON_AddItemToObject(g_index, id->valuestring, cJSON_Duplicate(data, 1));
    }

    /* Store DEEP COPY in collection to own the memory */
    cJSON_AddItemToArray(coll, cJSON_Duplicate(data, 1));

    _save_internal();
    pthread_mutex_unlock(&lock);
    return true;
}

/**
 * @brief Query documents from a collection.
 *
 * @param[in] coll_name Target collection name.
 * @param[in] query     JSON object defining query conditions.
 * @param[in] limit     Maximum number of documents to return.
 * @return cJSON* A new JSON array containing matched documents.
 */
cJSON *db_find(const char *coll_name, cJSON *query, int limit)
{
    pthread_mutex_lock(&lock);
    cJSON *result = cJSON_CreateArray();

    /* Fast Path: If query is specifically for an _id, use the index */
    cJSON *query_id = cJSON_GetObjectItem(query, "_id");
    if (query_id && cJSON_IsString(query_id)) {
        cJSON *found = cJSON_GetObjectItem(g_index, query_id->valuestring);
        if (found) {
            cJSON_AddItemToArray(result, cJSON_Duplicate(found, 1));
            pthread_mutex_unlock(&lock);
            return result;
        }
    }

    /* Slow Path: Linear scan */
    cJSON *coll = cJSON_GetObjectItem(root, coll_name);
    if (coll && cJSON_IsArray(coll)) {
        int count = 0;
        cJSON *item = coll->child; /* Manual iteration for safety */
        while (item) {
            if (limit > 0 && count >= limit)
                break;
            if (query_match(item, query)) {
                cJSON_AddItemToArray(result, cJSON_Duplicate(item, 1));
                count++;
            }
            item = item->next;
        }
    }
    pthread_mutex_unlock(&lock);
    return result;
}

/**
 * @brief Updates an existing document using Selective Merge Strategy.
 * * Supports partial updates. The _id field is immutable.
 * Uses a "Detach & Append" strategy to prevent SIGSEGV during high-concurrency access.
 *
 * @param[in] coll_name Target collection name.
 * @param[in] id        Document `_id` value (Immutable).
 * @param[in] data      JSON object containing fields to merge.
 * @return true if updated, false if the ID was not found.
 */
bool db_update(const char *coll_name, const char *id, cJSON *data)
{
    if (!data || !id)
        return false;

    pthread_mutex_lock(&lock);

    cJSON *coll = cJSON_GetObjectItem(root, coll_name);
    if (!coll || !cJSON_IsArray(coll)) {
        pthread_mutex_unlock(&lock);
        return false;
    }

    /* Iterate safely through the linked list */
    cJSON *existing_doc = coll->child;
    while (existing_doc) {
        cJSON *itemId = cJSON_GetObjectItem(existing_doc, "_id");

        /* Check ID match */
        if (itemId && cJSON_IsString(itemId) && strcmp(itemId->valuestring, id) == 0) {

            /* 1. Create a Deep Copy of the existing document (Memory Isolation) */
            cJSON *new_doc = cJSON_Duplicate(existing_doc, 1);
            if (!new_doc) {
                pthread_mutex_unlock(&lock);
                return false;
            }

            /* 2. Selective Merge on the Copy */
            cJSON *field = data->child;
            while (field) {
                if (field->string && strcmp(field->string, "_id") != 0) {
                    cJSON *dup_field = cJSON_Duplicate(field, 1);
                    if (cJSON_HasObjectItem(new_doc, field->string)) {
                        cJSON_ReplaceItemInObject(new_doc, field->string, dup_field);
                    } else {
                        cJSON_AddItemToObject(new_doc, field->string, dup_field);
                    }
                }
                field = field->next;
            }

            /* 3. Safe Swap Strategy: Detach old node, Append new node.
             * This prevents corruption of 'next/prev' pointers in the middle of the list. */
            cJSON_DetachItemViaPointer(coll, existing_doc);
            cJSON_Delete(existing_doc); /* Free old memory */

            cJSON_AddItemToArray(coll, new_doc); /* Append updated version to end */

            /* 4. Sync Index */
            cJSON_DeleteItemFromObject(g_index, id);
            cJSON_AddItemToObject(g_index, id, cJSON_Duplicate(new_doc, 1));

            _save_internal();
            pthread_mutex_unlock(&lock);
            return true;
        }

        existing_doc = existing_doc->next;
    }

    pthread_mutex_unlock(&lock);
    return false;
}

/**
 * @brief Updates an existing document or inserts it if not found.
 *
 * @param[in] coll_name Target collection name.
 * @param[in] id        Document `_id` value (optional for insert).
 * @param[in] data      JSON object representing the document.
 * @return true on success.
 */
bool db_upsert(const char *coll_name, const char *id, cJSON *data)
{
    /* If ID is provided, try updating first */
    if (id && db_update(coll_name, id, data)) {
        return true;
    }

    /* If update fails or ID is NULL, perform insert */
    return db_insert(coll_name, data);
}

/**
 * @brief Delete a document by its unique identifier.
 *
 * @param[in] coll_name Target collection name.
 * @param[in] id        Document `_id` value.
 * @return true if the document was found and deleted, false otherwise.
 */
bool db_delete(const char *coll_name, const char *id)
{
    pthread_mutex_lock(&lock);
    cJSON *coll = cJSON_GetObjectItem(root, coll_name);
    if (coll && cJSON_IsArray(coll)) {
        cJSON *item = coll->child;
        while (item) {
            cJSON *itemId = cJSON_GetObjectItem(item, "_id");
            if (itemId && cJSON_IsString(itemId) && strcmp(itemId->valuestring, id) == 0) {

                /* Safe deletion using detach */
                cJSON_DetachItemViaPointer(coll, item);
                cJSON_Delete(item);

                cJSON_DeleteItemFromObject(g_index, id);
                _save_internal();
                pthread_mutex_unlock(&lock);
                return true;
            }
            item = item->next;
        }
    }
    pthread_mutex_unlock(&lock);
    return false;
}

/**
 * @brief Counts documents in a collection.
 *
 * @param[in] coll_name Target collection name.
 * @return int Total document count.
 */
int db_count(const char *coll_name)
{
    pthread_mutex_lock(&lock);
    cJSON *coll = cJSON_GetObjectItem(root, coll_name);
    int cnt = (coll && cJSON_IsArray(coll)) ? cJSON_GetArraySize(coll) : 0;
    pthread_mutex_unlock(&lock);
    return cnt;
}
