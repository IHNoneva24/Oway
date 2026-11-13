#pragma once
#include "Game.h"
#include "Database.h"

const int LEADERBOARD_SIZE = 10;

// Save results
enum SaveResultStatus {
	SAVE_OK,
	SAVE_INVALID_NAME,
	SAVE_NOT_FINISHED,
	SAVE_DATABASE_ERROR
};

// Open leaderboard
bool InitLeaderboard(const char* databasePath);

// Close leaderboard
void ShutdownLeaderboard();

// Leaderboard ready?
bool IsLeaderboardAvailable();

// Name check
bool IsValidPlayerName(const char* name);

// Save result
SaveResultStatus SaveGameResult(const GameSession& session, const char* playerName, int& rank);

// Top results
int GetLeaderboard(int mazeSize, ResultRecord* results);
