#include "testFramework.h"
#include <iostream>

static int passedCount = 0;
static int failedCount = 0;

// Test group
void BeginTestGroup(const char* name) {
	std::cout << "\n=== " << name << " ===\n";
}

// Check
void Check(bool condition, const char* description) {
	if (condition) {
		passedCount++;
		std::cout << "  [PASS] " << description << "\n";
	}
	else {
		failedCount++;
		std::cout << "  [FAIL] " << description << "\n";
	}
}

// Check equal
void CheckEqual(int expected, int actual, const char* description) {
	Check(expected == actual, description);

	if (expected != actual) {
		std::cout << "         expected " << expected << ", got " << actual << "\n";
	}
}

// Test summary
int PrintTestSummary() {
	std::cout << "\n========================================\n";
	std::cout << "  Checks passed: " << passedCount << "\n";
	std::cout << "  Checks failed: " << failedCount << "\n";
	std::cout << "========================================\n";

	return failedCount;
}
