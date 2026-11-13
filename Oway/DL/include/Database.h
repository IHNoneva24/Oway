#pragma once

const int MAX_PLAYER_NAME_LENGTH = 16;

// Saved result
struct ResultRecord {
	char playerName[MAX_PLAYER_NAME_LENGTH + 1] = "";
	int mazeSize = 0;
	float timeSeconds = 0.0f;
	int moves = 0;
	char playedAt[20] = "";
};

// Open database
bool OpenDatabase(const char* filePath);

// Close database
void CloseDatabase();

// Database open?
bool IsDatabaseOpen();

// Insert result
bool InsertResult(const ResultRecord& result);

// Top results
int SelectTopResults(int mazeSize, ResultRecord* results, int maxCount);

// Result rank
int SelectRankForTime(int mazeSize, float timeSeconds);
