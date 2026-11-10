#include "tests.h"
#include "testFramework.h"
#include "Database.h"
#include <filesystem>
#include <string>
#include <cstring>

// The tests use their own database in the temp folder, so the real leaderboard is never changed
static std::string GetTestDatabasePath() {
	return (std::filesystem::temp_directory_path() / "oway_tests" / "database_test.db").string();
}

static ResultRecord MakeRecord(const char* name, int mazeSize, float timeSeconds, int moves) {
	ResultRecord record;
	strncpy_s(record.playerName, sizeof(record.playerName), name, _TRUNCATE);
	record.mazeSize = mazeSize;
	record.timeSeconds = timeSeconds;
	record.moves = moves;
	return record;
}

static void TestClosedDatabase() {
	CloseDatabase();
	ResultRecord results[3];

	Check(!IsDatabaseOpen(), "the database is closed at the start");
	Check(!InsertResult(MakeRecord("Test", 10, 5.0f, 10)), "nothing can be saved while the database is closed");
	CheckEqual(-1, SelectTopResults(10, results, 3), "reading a closed database returns an error (-1)");
	CheckEqual(-1, SelectRankForTime(10, 5.0f), "ranking with a closed database returns an error (-1)");
}

static void TestOpen() {
	std::filesystem::remove_all(std::filesystem::temp_directory_path() / "oway_tests");

	Check(OpenDatabase(GetTestDatabasePath().c_str()), "the database file and its folder are created");
	Check(IsDatabaseOpen(), "the database is open");
	Check(std::filesystem::exists(GetTestDatabasePath()), "the database file exists on the disk");
}

static void TestInsertAndSelect() {
	Check(InsertResult(MakeRecord("Slow", 15, 40.0f, 120)), "a result is saved");
	Check(InsertResult(MakeRecord("Fast", 15, 12.5f, 60)), "a second result is saved");
	Check(InsertResult(MakeRecord("Middle", 15, 20.0f, 80)), "a third result is saved");
	Check(InsertResult(MakeRecord("Other size", 20, 1.0f, 10)), "a result for another size is saved");

	ResultRecord results[10];
	int count = SelectTopResults(15, results, 10);

	CheckEqual(3, count, "only the results of the asked size are read");
	Check(count == 3 && strcmp(results[0].playerName, "Fast") == 0 && strcmp(results[2].playerName, "Slow") == 0,
		"the results are sorted from the fastest to the slowest");
	Check(count == 3 && results[0].moves == 60 && results[0].mazeSize == 15, "the moves and the size are read correctly");
	Check(count == 3 && strlen(results[0].playedAt) == 19, "the date is filled by the database (YYYY-MM-DD HH:MM:SS)");

	CheckEqual(2, SelectTopResults(15, results, 2), "maxCount limits the number of results");
	CheckEqual(-1, SelectTopResults(15, results, 0), "maxCount 0 is an error");

	CheckEqual(1, SelectRankForTime(15, 10.0f), "a time faster than all others is rank 1");
	CheckEqual(3, SelectRankForTime(15, 30.0f), "a time between the 2nd and the 3rd is rank 3");
	CheckEqual(1, SelectRankForTime(33, 99.0f), "the first result of a size is rank 1");
}

// The table rules (CHECK constraints) stop wrong data even if the BLL has a bug
static void TestConstraints() {
	Check(!InsertResult(MakeRecord("Too small", 5, 10.0f, 10)), "the database refuses a maze size under 10");
	Check(!InsertResult(MakeRecord("Too big", 51, 10.0f, 10)), "the database refuses a maze size over 50");
	Check(!InsertResult(MakeRecord("Negative", 10, -1.0f, 10)), "the database refuses a negative time");
	Check(!InsertResult(MakeRecord("Negative", 10, 1.0f, -5)), "the database refuses negative moves");

	// SQL injection attempt: the name is only data, the table must still exist
	Check(InsertResult(MakeRecord("x'); DROP TABLE Results;--", 10, 3.0f, 3)), "a name with SQL code is saved as normal text");

	ResultRecord results[10];
	Check(SelectTopResults(15, results, 10) == 3, "the table still exists after the SQL injection attempt");
}

void RunDatabaseTests() {
	BeginTestGroup("Database (DL/Database)");

	TestClosedDatabase();
	TestOpen();
	TestInsertAndSelect();
	TestConstraints();

	CloseDatabase();
	std::filesystem::remove_all(std::filesystem::temp_directory_path() / "oway_tests");
}
