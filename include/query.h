/**
 * @file query.h
 * @brief Logic engine for matching JSON documents against filters.
 *
 * This module provides the core filtering logic used by the database to
 * determine if a document meets specific search criteria.
 */

#ifndef QUERY_H
#define QUERY_H

#include "../third_party/cJSON/cJSON.h"

#include <stdbool.h>

/**
 * @brief Checks if a document matches a specific query filter.
 * * Performs a key-value comparison. For a match to be successful, all keys
 * present in the query must exist in the document with identical values.
 * * **Example:**
 * - Doc: `{"name": "Alice", "role": "admin"}`
 * - Query: `{"role": "admin"}`
 * - Result: `true`
 * * @param[in] doc   The source document cJSON object to check.
 * @param[in] query The filter criteria cJSON object.
 * @return true if the document matches all fields in the query, false otherwise.
 * @note This function performs a shallow match; nested objects may require
 * additional recursive handling depending on implementation.
 */
bool query_match(cJSON *doc, cJSON *query);

#endif /* QUERY_H */
