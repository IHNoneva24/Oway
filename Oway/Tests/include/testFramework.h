#pragma once

// Test group
void BeginTestGroup(const char* name);

// Check
void Check(bool condition, const char* description);

// Check equal
void CheckEqual(int expected, int actual, const char* description);

// Test summary
int PrintTestSummary();
