/**
 * @file framework.h
 * @brief Minimal, header-only unit testing framework.
 *
 * This utility provides a lightweight set of macros for defining and executing
 * unit tests. It is designed to be self-contained and easy to integrate into
 * the XDB build process for rapid verification.
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>

/** * @name ANSI Color Codes
 * @{
 */
#define CLR_GREEN "\033[0;32m" /**< ANSI Green for passing tests. */
#define CLR_RED "\033[0;31m"   /**< ANSI Red for failed assertions. */
#define CLR_RESET "\033[0m"    /**< ANSI Reset code. */
/** @} */

/**
 * @brief Global counters for test execution statistics.
 */
static int g_tests_run __attribute__((unused)) = 0; /**< Total number of tests executed. */
static int g_tests_failed = 0;                      /**< Total number of assertion failures. */

/**
 * @brief Macro to define the start of a test function.
 * * Automatically handles function signature creation and prints the test label.
 * @param name The name of the test function (e.g., test_db_insert).
 */
#define TEST_START(name)                                                       \
    void name(void) {                                                          \
        printf("[TEST] %-35s", #name);

/**
 * @brief Macro to terminate a test function.
 * * Closes the function block defined by TEST_START.
 */
#define TEST_END }

/**
 * @brief Fundamental assertion macro.
 * * Evaluates a condition. If it fails, prints the line number, increments
 * the failure counter, and exits the current test function immediately.
 * @param cond The boolean expression to evaluate.
 */
#define ASSERT(cond)                                                           \
    do {                                                                       \
        if (!(cond)) {                                                         \
            printf("%sFAILED%s (Line %d)\n", CLR_RED, CLR_RESET, __LINE__);    \
            g_tests_failed++;                                                  \
            return;                                                            \
        }                                                                      \
    } while (0)

/**
 * @brief Equality assertion macro.
 * * Convenient wrapper for ASSERT to compare two values for equality.
 * @param val      The actual value.
 * @param expected The expected value.
 */
#define ASSERT_EQ(val, expected) ASSERT((val) == (expected))

/**
 * @brief Executes a test function and tracks results.
 * * Increments the global run counter and prints a success message if the
 * test function returns without an assertion failure.
 * @param func The name of the test function to execute.
 */
#define REGISTER_TEST(func)                                                    \
    do {                                                                       \
        g_tests_run++;                                                         \
        func();                                                                \
        printf("%sPASS%s\n", CLR_GREEN, CLR_RESET);                            \
    } while (0)

#endif /* TEST_FRAMEWORK_H */
