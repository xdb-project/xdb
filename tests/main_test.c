/**
 * @file main_test.c
 * @brief XDB unit test entry point.
 *
 * This module coordinates the execution of the unit test suite. It initializes
 * an isolated test environment, executes registered test cases, and provides
 * a summary report of passing and failing assertions.
 */

#include "../include/database.h"
#include "framework.h"

#include <stdio.h>

/**
 * @brief Query engine exact match test.
 * @note Implementation located in test_query.c.
 */
void test_query_exact_match(void);

/**
 * @brief Full CRUD workflow test.
 * @note Implementation located in test_crud.c.
 */
void test_crud_workflow(void);

/**
 * @brief Test runner entry point.
 * * Sets up a temporary database file, executes all registered unit tests,
 * and performs cleanup. The exit code reflects the success or failure
 * of the entire suite.
 *
 * @return int 0 if all tests passed, 1 if any assertions failed.
 */
int main(void)
{
    printf("XDB Unit Test Suite Execution");

    /* 1. Initialize isolated test database to protect production data */
    db_init("data/test_db.json");

    /* 2. Ensure a clean state before starting the suite */
    db_drop_all();

    /* 3. Execute Query Logic Tests */
    REGISTER_TEST(test_query_exact_match);

    /* 4. Reset database state to isolate test side-effects */
    db_drop_all();

    /* 5. Execute CRUD Workflow Tests */
    REGISTER_TEST(test_crud_workflow);

    /* 6. Cleanup database memory resources */
    db_cleanup();

    /* 7. Remove the physical test file to leave no trace */
    remove("data/test_db.json");

    /* 8. Final Report */
    printf("Result: %d Run, %d Failed.\n", g_tests_run, g_tests_failed);

    return (g_tests_failed > 0) ? 1 : 0;
}
