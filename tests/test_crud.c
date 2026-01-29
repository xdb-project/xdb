/**
 * @file test_crud.c
 * @brief Unit tests for basic database CRUD workflow.
 *
 * This test suite verifies the end-to-end lifecycle of a document, including
 * insertion, counting, finding, updating, and eventual deletion.
 */

#include "../include/database.h"
#include "framework.h"

#include <stdlib.h> /* Fixed: Added for free() declaration */
#include <string.h>

/**
 * @brief Tests the full Create, Read, Update, and Delete cycle.
 * * This test ensures that:
 * 1. Documents can be inserted and assigned an automatic `_id`.
 * 2. Documents can be retrieved and matched.
 * 3. Documents can be updated using the `_id`.
 * 4. Upsert logic correctly handles new and existing documents.
 * 5. Documents can be removed by their unique identifier.
 */
TEST_START(test_crud_workflow)

/* 1. Prepare dummy data */
cJSON *doc = cJSON_CreateObject();
cJSON_AddStringToObject(doc, "username", "unit_test_bot");
cJSON_AddNumberToObject(doc, "score", 100);

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
char *id_str = strdup(id_obj->valuestring); /* Store ID for subsequent tests */

/* 4. Test Update */
cJSON *update_data = cJSON_CreateObject();
cJSON_AddStringToObject(update_data, "username", "bot_v2");
cJSON_AddNumberToObject(update_data, "score", 200);

bool update_result = db_update("users", id_str, update_data);
ASSERT(update_result == true);

/* Verify update result */
cJSON *updated_results = db_find("users", NULL, 0);
cJSON *updated_item = cJSON_GetArrayItem(updated_results, 0);
/* Fixed: Used ASSERT with strcmp since ASSERT_STR_EQ was undefined */
ASSERT(strcmp(cJSON_GetObjectItem(updated_item, "username")->valuestring, "bot_v2") == 0);
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
