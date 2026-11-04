#pragma once

// SQLite database access - the only place in the game that talks to the database

const int MAX_PLAYER_NAME_LENGTH = 16;

struct ResultRecord {
	char playerName[MAX_PLAYER_NAME_LENGTH + 1] = "";
	int mazeSize = 0;
	float timeSeconds = 0.0f;
	int moves = 0;
	char playedAt[20] = ""; // "YYYY-MM-DD HH:MM:SS", filled by the database
};

// Opens (or creates) the database file, its folder and the tables. Returns false on error
bool OpenDatabase(const char* filePath);

void CloseDatabase();

bool IsDatabaseOpen();

// Saves one finished game. Returns false on error
bool InsertResult(const ResultRecord& result);

// Fills "results" with the fastest games for one maze size (fewest moves on equal time).
// Returns how many records were written (0 - maxCount), or -1 on error
int SelectTopResults(int mazeSize, ResultRecord* results, int maxCount);

// Returns the position (1 = best) the given time would have for this maze size, or -1 on error
int SelectRankForTime(int mazeSize, float timeSeconds);
