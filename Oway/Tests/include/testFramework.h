#pragma once

// A very small test framework: every check prints PASS or FAIL and the results are counted

// Starts a new group of tests (one tested module), printed as a heading
void BeginTestGroup(const char* name);

// Checks one condition. "description" says what is expected, e.g. "size 9 is rejected"
void Check(bool condition, const char* description);

// Checks that two whole numbers are equal and prints both if they are not
void CheckEqual(int expected, int actual, const char* description);

// Prints how many checks passed and failed. Returns the number of failed checks
int PrintTestSummary();
