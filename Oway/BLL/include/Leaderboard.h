#pragma once
#include "Game.h"
#include "Database.h"

// Leaderboard logic - validates the data and passes it to the DL

const int LEADERBOARD_SIZE = 10;

enum SaveResultStatus {
	SAVE_OK,
	SAVE_INVALID_NAME,
	SAVE_NOT_FINISHED,
	SAVE_DATABASE_ERROR
};

// Opens the database at the given path. Returns false if it can't be opened
bool InitLeaderboard(const char* databasePath);

void ShutdownLeaderboard();

bool IsLeaderboardAvailable();

// A valid name has 1-16 characters: letters, digits, spaces, '_' or '-' and is not only spaces
bool IsValidPlayerName(const char* name);

// Saves the result of a finished session. On success "rank" gets the place (1 = best)
SaveResultStatus SaveGameResult(const GameSession& session, const char* playerName, int& rank);

// Fills "results" with the best LEADERBOARD_SIZE results for a maze size. Returns the count (0 on error)
int GetLeaderboard(int mazeSize, ResultRecord* results);
