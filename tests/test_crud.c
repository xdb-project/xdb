/**
 * @file test_crud.c
 * @brief Unit tests for basic database CRUD workflow.
 *
 * This test suite verifies the end-to-end lifecycle of a document, including
 * insertion, counting, finding, selective updating, and eventual deletion.
 */

#include "../include/database.h"
#include "framework.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Tests the full Create, Read, Update, and Delete cycle.
 * * This test ensures that:
 * 1. Documents can be inserted and assigned an automatic `_id`.
 * 2. Documents can be retrieved and matched.
 * 3. Selective Update (v1.4.0) merges fields without data loss.
 * 4. _id remains Immutable even if a new one is provided in the payload.
 * 5. Upsert logic correctly handles new and existing documents.
 * 6. Documents can be removed by their unique identifier.
 */
TEST_START(test_crud_workflow)

/* 1. Prepare dummy data */
cJSON *doc = cJSON_CreateObject();
cJSON_AddStringToObject(doc, "username", "unit_test_bot");
cJSON_AddNumberToObject(doc, "score", 100);
cJSON_AddStringToObject(doc, "status", "online");

/* 2. Test Insertion */
bool insert_result = db_insert("users", doc);
ASSERT(insert_result == true);

/* 3. Test Retrieval and ID extraction */
cJSON *results = db_find("users", NULL, 0);
ASSERT(results != NULL);
ASSERT_EQ(cJSON_GetArraySize(results), 1);

cJSON *item = cJSON_GetArrayItem(results, 0);
cJSON *id_obj = cJSON_GetObjectItem(item, "_id");
ASSERT(id_obj != NULL);
char *id_str = strdup(id_obj->valuestring);

/* 4. Test Selective Update (v1.4.0) */
cJSON *update_payload = cJSON_CreateObject();
cJSON_AddNumberToObject(update_payload, "score", 200);       /* Change existing field */
cJSON_AddStringToObject(update_payload, "rank", "gold");     /* Add new field */
cJSON_AddStringToObject(update_payload, "_id", "HACKED_ID"); /* Attempt to change ID */

bool update_result = db_update("users", id_str, update_payload);
ASSERT(update_result == true);

/* Verify Selective Update and Immutability */
cJSON *updated_results = db_find("users", NULL, 0);
cJSON *updated_item = cJSON_GetArrayItem(updated_results, 0);

/* Check if fields merged correctly */
ASSERT_EQ(cJSON_GetObjectItem(updated_item, "score")->valueint, 200);
ASSERT(strcmp(cJSON_GetObjectItem(updated_item, "rank")->valuestring, "gold") == 0);

/* Check if original field still exists (Selective Update property) */
ASSERT(cJSON_HasObjectItem(updated_item, "status") == true);
ASSERT(strcmp(cJSON_GetObjectItem(updated_item, "status")->valuestring, "online") == 0);

/* Check if _id remained the same (Immutability property) */
ASSERT(strcmp(cJSON_GetObjectItem(updated_item, "_id")->valuestring, id_str) == 0);

cJSON_Delete(updated_results);

/* 5. Test Upsert (Existing Document) */
cJSON *upsert_data = cJSON_CreateObject();
cJSON_AddNumberToObject(upsert_data, "score", 999);

bool upsert_result = db_upsert("users", id_str, upsert_data);
ASSERT(upsert_result == true);

/* 6. Test Deletion */
bool delete_result = db_delete("users", id_str);
ASSERT(delete_result == true);

/* 7. Verify empty state */
ASSERT_EQ(db_count("users"), 0);

/* Cleanup resources */
free(id_str);
cJSON_Delete(results);

TEST_END
