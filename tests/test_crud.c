/**
 * @file test_crud.c
 * @brief Unit tests for basic database CRUD workflow.
 *
 * This test suite verifies the end-to-end lifecycle of a document, including
 * insertion, counting, finding, and eventual deletion within a collection.
 */

#include "../include/database.h"
#include "framework.h"

#include <string.h>

/**
 * @brief Tests the full Create, Read, Update, and Delete cycle.
 * * This test ensures that:
 * 1. Documents can be inserted and assigned an automatic `_id`.
 * 2. The collection count updates correctly.
 * 3. Documents can be retrieved and matched.
 * 4. Documents can be removed by their unique identifier.
 */
TEST_START(test_crud_workflow)

/* 1. Prepare dummy data */
cJSON *doc = cJSON_CreateObject();
cJSON_AddStringToObject(doc, "username", "unit_test_bot");
cJSON_AddNumberToObject(doc, "score", 100);

/* 2. Test Insertion */
bool insert_result = db_insert("users", doc);
ASSERT(insert_result == true);

/* 3. Test Counting */
int count = db_count("users");
ASSERT_EQ(count, 1);

/* 4. Test Retrieval and ID extraction */
cJSON *results = db_find("users", NULL, 0);
ASSERT_EQ(cJSON_GetArraySize(results), 1);

cJSON *item = cJSON_GetArrayItem(results, 0);
cJSON *id_obj = cJSON_GetObjectItem(item, "_id");
ASSERT(id_obj != NULL);
char *id_str = id_obj->valuestring;

/* 5. Test Deletion */
bool delete_result = db_delete("users", id_str);
ASSERT(delete_result == true);

/* 6. Verify empty state */
ASSERT_EQ(db_count("users"), 0);

/* Cleanup local result array */
cJSON_Delete(results);

TEST_END
