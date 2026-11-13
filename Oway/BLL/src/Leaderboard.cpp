#include "Leaderboard.h"
#include <cstring>

// Allowed character
static bool IsAllowedNameCharacter(char character) {
	return (character >= 'a' && character <= 'z')
		|| (character >= 'A' && character <= 'Z')
		|| (character >= '0' && character <= '9')
		|| character == ' ' || character == '_' || character == '-';
}

// Open leaderboard
bool InitLeaderboard(const char* databasePath) {
	return OpenDatabase(databasePath);
}

// Close leaderboard
void ShutdownLeaderboard() {
	CloseDatabase();
}

// Leaderboard ready?
bool IsLeaderboardAvailable() {
	return IsDatabaseOpen();
}

// Name check
bool IsValidPlayerName(const char* name) {
	if (name == nullptr) {
		return false;
	}

	int length = 0;
	bool hasVisibleCharacter = false;

	for (; name[length] != '\0'; length++) {
		if (length >= MAX_PLAYER_NAME_LENGTH || !IsAllowedNameCharacter(name[length])) {
			return false;
		}

		if (name[length] != ' ') {
			hasVisibleCharacter = true;
		}
	}

	return length > 0 && hasVisibleCharacter;
}

// Save result
SaveResultStatus SaveGameResult(const GameSession& session, const char* playerName, int& rank) {
	if (!session.isFinished) {
		return SAVE_NOT_FINISHED;
	}

	if (!IsValidPlayerName(playerName)) {
		return SAVE_INVALID_NAME;
	}

	ResultRecord record;
	// safe copy
	strncpy_s(record.playerName, sizeof(record.playerName), playerName, _TRUNCATE);
	record.mazeSize = session.maze.width;
	record.timeSeconds = session.elapsedSeconds;
	record.moves = session.player.moves;

	int newRank = SelectRankForTime(record.mazeSize, record.timeSeconds);

	if (newRank < 0 || !InsertResult(record)) {
		return SAVE_DATABASE_ERROR;
	}

	rank = newRank;
	return SAVE_OK;
}

// Top results
int GetLeaderboard(int mazeSize, ResultRecord* results) {
	if (!IsValidMazeSize(mazeSize)) {
		return 0;
	}

	int count = SelectTopResults(mazeSize, results, LEADERBOARD_SIZE);
	return count < 0 ? 0 : count;
}
