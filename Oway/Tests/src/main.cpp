#include "tests.h"
#include "testFramework.h"
#include <iostream>

// Runs every test of the BLL and the DL. The exit code is the number of failed checks (0 = everything passed)
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
