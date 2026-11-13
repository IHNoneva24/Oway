#include "tests.h"
#include "testFramework.h"
#include <iostream>

// Entry point
int main() {
    std::cout << "Oway - automated tests\n";

    RunMazeTests();
    RunGameTests();
    RunRaceTests();
    RunTeamTests();
    RunDatabaseTests();
    RunLeaderboardTests();
    RunNetworkTests();

    return PrintTestSummary();
}
