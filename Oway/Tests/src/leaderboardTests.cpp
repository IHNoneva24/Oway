#include "tests.h"
#include "testFramework.h"
#include "Leaderboard.h"
#include <filesystem>
#include <string>

static std::string GetTestDatabasePath() {
	return (std::filesystem::temp_directory_path() / "oway_tests" / "leaderboard_test.db").string();
}

static void TestNameValidation() {
	Check(IsValidPlayerName("Galin"), "\"Galin\" is a valid name");
	Check(IsValidPlayerName("Team_2 - A"), "letters, digits, spaces, _ and - are allowed");
	Check(IsValidPlayerName("1234567890123456"), "16 characters are allowed");
	Check(!IsValidPlayerName("12345678901234567"), "17 characters are too many");
	Check(!IsValidPlayerName(""), "an empty name is rejected");
	Check(!IsValidPlayerName("    "), "a name of only spaces is rejected");
	Check(!IsValidPlayerName("bad;name"), "';' is not allowed");
	Check(!IsValidPlayerName("O'Brien"), "a quote is not allowed");
	Check(!IsValidPlayerName(nullptr), "a missing name (nullptr) is rejected");
}

// A finished single player game with the given result
static void FinishGame(GameSession& session, int size, float timeSeconds, int moves) {
	StartGameSession(session, size, 1u);
	session.elapsedSeconds = timeSeconds;
	session.player.moves = moves;
	session.isFinished = true;
}

static void TestSaving() {
	std::filesystem::remove_all(std::filesystem::temp_directory_path() / "oway_tests");
	Check(InitLeaderboard(GetTestDatabasePath().c_str()), "the leaderboard opens its database");
	Check(IsLeaderboardAvailable(), "the leaderboard is available");

	GameSession session;
	int rank = 0;

	StartGameSession(session, 12, 1u);
	Check(SaveGameResult(session, "Galin", rank) == SAVE_NOT_FINISHED, "an unfinished game can't be saved");

	FinishGame(session, 12, 30.0f, 90);
	Check(SaveGameResult(session, "bad;name", rank) == SAVE_INVALID_NAME, "a wrong name is not saved");

	Check(SaveGameResult(session, "First", rank) == SAVE_OK, "a finished game with a good name is saved");
	CheckEqual(1, rank, "the first result is rank 1");

	FinishGame(session, 12, 20.0f, 70);
	SaveGameResult(session, "Faster", rank);
	CheckEqual(1, rank, "a faster time becomes the new rank 1");

	FinishGame(session, 12, 50.0f, 150);
	SaveGameResult(session, "Slowest", rank);
	CheckEqual(3, rank, "the slowest time is rank 3");

	ResultRecord results[LEADERBOARD_SIZE];
	int count = GetLeaderboard(12, results);
	CheckEqual(3, count, "the leaderboard for 12x12 has 3 results");
	Check(count == 3 && results[0].timeSeconds < results[1].timeSeconds && results[1].timeSeconds < results[2].timeSeconds,
		"the leaderboard is sorted by time");
	CheckEqual(0, GetLeaderboard(7, results), "a wrong maze size gives an empty leaderboard");

	EndGameSession(session);
	ShutdownLeaderboard();
	Check(!IsLeaderboardAvailable(), "the leaderboard is closed after shutdown");

	GameSession finished;
	FinishGame(finished, 12, 10.0f, 10);
	Check(SaveGameResult(finished, "Late", rank) == SAVE_DATABASE_ERROR, "saving without a database reports an error");
	EndGameSession(finished);

	std::filesystem::remove_all(std::filesystem::temp_directory_path() / "oway_tests");
}

void RunLeaderboardTests() {
	BeginTestGroup("Leaderboard (BLL/Leaderboard)");

	TestNameValidation();
	TestSaving();
}
