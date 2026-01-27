/**
 * @file test_query.c
 * @brief Unit tests for query matching logic.
 *
 * This test suite validates the logic engine responsible for comparing
 * database documents against JSON filter criteria, ensuring that exact
 * matches succeed and mismatches are correctly identified.
 */

#include "../include/query.h"
#include "../third_party/cJSON/cJSON.h"
#include "framework.h"

/**
 * @brief Tests basic key-value matching for strings and numbers.
 * * This test ensures that:
 * 1. A document containing the query key-value pair returns true.
 * 2. A document with a different value for the same key returns false.
 * 3. Memory is correctly managed for local test objects.
 */
TEST_START(test_query_exact_match)

/* 1. Setup test document */
cJSON *doc = cJSON_CreateObject();
cJSON_AddStringToObject(doc, "name", "X");
cJSON_AddNumberToObject(doc, "version", 1);

/* 2. Setup matching query */
cJSON *q_match = cJSON_CreateObject();
cJSON_AddStringToObject(q_match, "name", "X");

/* 3. Setup failing query */
cJSON *q_fail = cJSON_CreateObject();
cJSON_AddNumberToObject(q_fail, "version", 99);

/* 4. Execute assertions */
ASSERT(query_match(doc, q_match) == true);
ASSERT(query_match(doc, q_fail) == false);

/* 5. Cleanup */
cJSON_Delete(doc);
cJSON_Delete(q_match);
cJSON_Delete(q_fail);

TEST_END
