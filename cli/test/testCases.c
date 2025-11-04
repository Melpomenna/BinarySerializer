#include "testCases.h"

#define DECLARE_CASE(__fpath, __spath, __rpath, __in1, __in2, __out)           \
  {                                                                            \
    __fpath, __spath, __rpath, __in1, __in2, __out,                            \
        sizeof(__in1) / sizeof(StatData), sizeof(__in2) / sizeof(StatData),    \
        sizeof(__out) / sizeof(StatData)                                       \
  }

#pragma region TestCase1Base
static StatData testCase1[] = {
    {.id = 90889, .count = 13, .cost = 3.567, .primary = 0, .mode = 3},
    {.id = 90089, .count = 1, .cost = 88.90, .primary = 1, .mode = 0}};
static StatData testCase2[] = {
    {.id = 90089, .count = 13, .cost = 0.011, .primary = 0, .mode = 2},
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2}};

static StatData testCaseResult1[] = {
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2},
    {.id = 90889, .count = 13, .cost = 3.567, .primary = 0, .mode = 3},
    {.id = 90089, .count = 14, .cost = 88.911, .primary = 0, .mode = 2}};

#pragma endregion

#pragma region TestCase2Base
static StatData testCase12[] = {
    {.id = 90000, .count = 13, .cost = 0, .primary = 0, .mode = 3},
    {.id = 90001, .count = 1, .cost = 1, .primary = 1, .mode = 0},
    {.id = 90002, .count = 13, .cost = 2, .primary = 0, .mode = 3},
    {.id = 90003, .count = 1, .cost = 3, .primary = 1, .mode = 0},
    {.id = 90004, .count = 13, .cost = 4, .primary = 0, .mode = 3},
    {.id = 90005, .count = 1, .cost = 5, .primary = 1, .mode = 0},
    {.id = 90006, .count = 13, .cost = 6, .primary = 0, .mode = 3},
    {.id = 90007, .count = 1, .cost = 7, .primary = 1, .mode = 0},
    {.id = 90808, .count = 13, .cost = 8, .primary = 0, .mode = 3},
    {.id = 90009, .count = 1, .cost = 9, .primary = 1, .mode = 0},
    {.id = 90810, .count = 13, .cost = 10, .primary = 0, .mode = 3},
    {.id = 90011, .count = 1, .cost = 11, .primary = 1, .mode = 0},
    {.id = 90812, .count = 13, .cost = 12, .primary = 0, .mode = 3},
    {.id = 90013, .count = 1, .cost = 13, .primary = 1, .mode = 0},
    {.id = 90814, .count = 13, .cost = 14, .primary = 0, .mode = 3},
    {.id = 90015, .count = 1, .cost = 15, .primary = 1, .mode = 0},
    {.id = 90816, .count = 13, .cost = 16, .primary = 0, .mode = 3},
    {.id = 90017, .count = 1, .cost = 17, .primary = 1, .mode = 0},
    {.id = 90818, .count = 13, .cost = 18, .primary = 0, .mode = 3},
    {.id = 90019, .count = 1, .cost = 19, .primary = 1, .mode = 0},
    {.id = 90820, .count = 13, .cost = 20, .primary = 0, .mode = 3},
    {.id = 90021, .count = 1, .cost = 21, .primary = 1, .mode = 0},
    {.id = 90822, .count = 13, .cost = 22, .primary = 0, .mode = 3},
    {.id = 90023, .count = 1, .cost = 23, .primary = 1, .mode = 0},
    {.id = 90824, .count = 13, .cost = 24, .primary = 0, .mode = 3},
    {.id = 90025, .count = 1, .cost = 25, .primary = 1, .mode = 0},
    {.id = 90826, .count = 13, .cost = 26, .primary = 0, .mode = 3},
    {.id = 90027, .count = 1, .cost = 27, .primary = 1, .mode = 0},
    {.id = 90828, .count = 13, .cost = 28, .primary = 0, .mode = 3},
    {.id = 90029, .count = 1, .cost = 29, .primary = 1, .mode = 0},
    {.id = 90830, .count = 13, .cost = 30, .primary = 0, .mode = 3},
    {.id = 90031, .count = 1, .cost = 31, .primary = 1, .mode = 0},
    {.id = 90832, .count = 13, .cost = 32, .primary = 0, .mode = 3},
    {.id = 90033, .count = 1, .cost = 33, .primary = 1, .mode = 0},
    {.id = 90834, .count = 13, .cost = 34, .primary = 0, .mode = 3},
    {.id = 90035, .count = 1, .cost = 35, .primary = 1, .mode = 0},
    {.id = 90836, .count = 13, .cost = 36, .primary = 0, .mode = 3},
    {.id = 90037, .count = 1, .cost = 37, .primary = 1, .mode = 0},
    {.id = 90838, .count = 13, .cost = 38, .primary = 0, .mode = 3},
    {.id = 90039, .count = 1, .cost = 39, .primary = 1, .mode = 0},
    {.id = 90840, .count = 13, .cost = 40, .primary = 0, .mode = 3},
    {.id = 90041, .count = 1, .cost = 41, .primary = 1, .mode = 0},
    {.id = 90842, .count = 13, .cost = 42, .primary = 0, .mode = 3},
    {.id = 90043, .count = 1, .cost = 43, .primary = 1, .mode = 0},
    {.id = 90844, .count = 13, .cost = 44, .primary = 0, .mode = 3},
    {.id = 90045, .count = 1, .cost = 45, .primary = 1, .mode = 0},
    {.id = 90846, .count = 13, .cost = 46, .primary = 0, .mode = 3},
    {.id = 90047, .count = 1, .cost = 47, .primary = 1, .mode = 0},
    {.id = 90848, .count = 13, .cost = 48, .primary = 0, .mode = 3},
    {.id = 90049, .count = 1, .cost = 49, .primary = 1, .mode = 0},
    {.id = 90850, .count = 13, .cost = 50, .primary = 0, .mode = 3},
    {.id = 90051, .count = 1, .cost = 51, .primary = 1, .mode = 0},
    {.id = 90852, .count = 13, .cost = 52, .primary = 0, .mode = 3},
    {.id = 90053, .count = 1, .cost = 53, .primary = 1, .mode = 0},
    {.id = 90854, .count = 13, .cost = 54, .primary = 0, .mode = 3},
    {.id = 90055, .count = 1, .cost = 55, .primary = 1, .mode = 0},
    {.id = 90856, .count = 13, .cost = 56, .primary = 0, .mode = 3},
    {.id = 90057, .count = 1, .cost = 57, .primary = 1, .mode = 0},
    {.id = 90858, .count = 13, .cost = 58, .primary = 0, .mode = 3},
    {.id = 90059, .count = 1, .cost = 59, .primary = 1, .mode = 0},
    {.id = 90860, .count = 13, .cost = 60, .primary = 0, .mode = 3},
    {.id = 90061, .count = 1, .cost = 61, .primary = 1, .mode = 0},
    {.id = 90862, .count = 13, .cost = 62, .primary = 0, .mode = 3},
    {.id = 90063, .count = 1, .cost = 63, .primary = 1, .mode = 0},
    {.id = 90864, .count = 13, .cost = 64, .primary = 0, .mode = 3},
    {.id = 90065, .count = 1, .cost = 65, .primary = 1, .mode = 0},
    {.id = 90866, .count = 13, .cost = 66, .primary = 0, .mode = 3},
    {.id = 90067, .count = 1, .cost = 67, .primary = 1, .mode = 0},
    {.id = 90868, .count = 13, .cost = 68, .primary = 0, .mode = 3},
    {.id = 90069, .count = 1, .cost = 69, .primary = 1, .mode = 0},
    {.id = 90870, .count = 13, .cost = 70, .primary = 0, .mode = 3},
    {.id = 90071, .count = 1, .cost = 71, .primary = 1, .mode = 0},
    {.id = 90872, .count = 13, .cost = 72, .primary = 0, .mode = 3},
    {.id = 90073, .count = 1, .cost = 73, .primary = 1, .mode = 0},
    {.id = 90874, .count = 13, .cost = 74, .primary = 0, .mode = 3},
    {.id = 90075, .count = 1, .cost = 75, .primary = 1, .mode = 0},
    {.id = 90077, .count = 1, .cost = 76, .primary = 1, .mode = 0},
    {.id = 90878, .count = 13, .cost = 77, .primary = 0, .mode = 3},
    {.id = 90079, .count = 1, .cost = 78, .primary = 1, .mode = 0},
    {.id = 90876, .count = 13, .cost = 79, .primary = 0, .mode = 3},
};

#pragma endregion

#pragma region TestCase3Base
static StatData testCase3[] = {
    {.id = 90889, .count = 13, .cost = 100, .primary = 1, .mode = 3},
    {.id = 90089, .count = 1, .cost = 88.90, .primary = 0, .mode = 5}};
static StatData testCase31[] = {
    {.id = 90089, .count = 13, .cost = 0.1, .primary = 1, .mode = 2},
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2}};

static StatData testCaseResult3[] = {
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2},
    {.id = 90089, .count = 14, .cost = 89, .primary = 0, .mode = 5},
    {.id = 90889, .count = 13, .cost = 100, .primary = 1, .mode = 3},
};

#pragma endregion

#pragma region TestCase4Base
static StatData testCase4[] = {
    {.id = 90889, .count = 13, .cost = 100, .primary = 1, .mode = 3},
    {.id = 90089, .count = 1, .cost = 88.90, .primary = 0, .mode = 5}};
static StatData testCase41[] = {
    {.id = 90089, .count = 13, .cost = 0.1, .primary = 1, .mode = 7},
    {.id = 90089, .count = 13, .cost = 0, .primary = 1, .mode = 4},
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2}};

static StatData testCaseResult4[] = {
    {.id = 90189, .count = 1000, .cost = 1.00003, .primary = 1, .mode = 2},
    {.id = 90089, .count = 27, .cost = 89, .primary = 0, .mode = 7},
    {.id = 90889, .count = 13, .cost = 100, .primary = 1, .mode = 3},
};

#pragma endregion

#pragma region TestCase5Base
static StatData testCase5[] = {
    {.id = 90189, .count = 1, .cost = -8.008, .primary = 1, .mode = 3},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 0, .mode = 5},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 1, .mode = 3},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 0, .mode = 5},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 1, .mode = 3},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 0, .mode = 5}};
static StatData testCase51[] = {
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 1, .mode = 7},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 1, .mode = 4},
    {.id = 90189, .count = 1, .cost = 1.001, .primary = 1, .mode = 2}};

static StatData testCaseResult5[] = {
    {.id = 90189, .count = 9, .cost = 0, .primary = 0, .mode = 7}};

#pragma endregion

static TestCase cases[] = {
    DECLARE_CASE("case_1_in_a.dat", "case_1_in_b.dat", "case_1_out.dat",
                 testCase1, testCase2, testCaseResult1), // SUCCESS TestCase 0
    DECLARE_CASE("case_2_in_a.dat", "case_2_in_b.dat", "case_2_out.dat",
                 testCase12, NULL, testCase12), // SUCCESS TestCase 1
    DECLARE_CASE(NULL, NULL, "case_2_out.dat", testCase12, NULL,
                 testCase12), // Must failed TestCase2
    DECLARE_CASE("case_2_in_a.dat", "case_2_in_b.dat", "case_2_out.dat", NULL,
                 NULL, testCase12),                   //  Must failed TestCase 3
    DECLARE_CASE(NULL, NULL, NULL, NULL, NULL, NULL), // Must failed TestCase 4
    DECLARE_CASE("case_3_in_a.dat", "case_3_in_b.dat", "case_3_out.dat",
                 testCase3, testCase31, testCaseResult3), // SUCCESS TestCase 5
    DECLARE_CASE("case_1_in_a.dat", "case_1_in_b.dat", "case_1_out.dat",
                 testCase1, testCase2, NULL), // Must failed TestCase 6
    DECLARE_CASE("case_4_in_a.dat", "case_4_in_b.dat", "case_4_out.dat",
                 testCase4, testCase41, testCaseResult4), // SUCCESS TestCase 7
    DECLARE_CASE("case_5_in_a.dat", "case_5_in_b.dat", "case_5_out.dat",
                 testCase5, testCase51, testCaseResult5), // SUCCESS TestCase 8
};

TestCase *GetTestCase(size_t id) { return &cases[id]; }

size_t GetTestsCount() { return sizeof(cases) / sizeof(TestCase); }