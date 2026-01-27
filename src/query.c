/**
 * @file query.c
 * @brief Query matching engine for the Database system.
 *
 * This module provides the implementation for comparing database documents
 * against specific JSON filter criteria. It serves as the primary filtering
 * logic for the find and delete operations.
 */

#include "../include/query.h"

#include <string.h>

/**
 * @brief Evaluates if a document matches a given query filter.
 * * Performs a field-by-field comparison between the document and the query.
 * Supported types for matching include Strings, Numbers, and Booleans.
 * For a match to be successful, every key in the query must exist in the
 * document with a matching value and type.
 * * @param[in] doc   The source JSON document to evaluate.
 * @param[in] query The JSON object containing filter criteria (NULL matches all).
 * @return true if the document satisfies all query conditions or if query is NULL.
 * @return false if any field value/type differs or if a required field is missing.
 * @note This implementation performs shallow matching; nested objects or
 * arrays are treated as type mismatches.
 */
bool query_match(cJSON *doc, cJSON *query)
{
    /* If query is NULL, it acts as a wildcard (Match All) */
    if (!query) {
        return true;
    }

    /* If query exists but document is NULL, no match is possible */
    if (!doc) {
        return false;
    }

    cJSON *item = NULL;

    /* Iterate through every field defined in the query object */
    cJSON_ArrayForEach(item, query)
    {
        cJSON *doc_val = cJSON_GetObjectItem(doc, item->string);

        /* 1. Check if the required field exists in the document */
        if (!doc_val) {
            return false;
        }

        /* 2. Compare based on JSON type */

        /* String Comparison */
        if (cJSON_IsString(item) && cJSON_IsString(doc_val)) {
            if (strcmp(item->valuestring, doc_val->valuestring) != 0) {
                return false;
            }
        }
        /* Number Comparison (Double Precision) */
        else if (cJSON_IsNumber(item) && cJSON_IsNumber(doc_val)) {
            if (item->valuedouble != doc_val->valuedouble) {
                return false;
            }
        }
        /* Boolean Comparison */
        else if (cJSON_IsBool(item) && cJSON_IsBool(doc_val)) {
            if (cJSON_IsTrue(item) != cJSON_IsTrue(doc_val)) {
                return false;
            }
        }
        /* Mismatching types or unsupported types (Arrays/Objects) */
        else {
            return false;
        }
    }

    /* All query conditions were satisfied */
    return true;
}
