#include "Database.h"
#include "sqlite3.h"
#include <cstring>
#include <filesystem>

static sqlite3* database = nullptr;

static const char* CREATE_TABLES_SQL =
	"CREATE TABLE IF NOT EXISTS Results ("
	"  Id          INTEGER PRIMARY KEY AUTOINCREMENT,"
	"  PlayerName  TEXT    NOT NULL,"
	"  MazeSize    INTEGER NOT NULL CHECK (MazeSize BETWEEN 10 AND 50),"
	"  TimeSeconds REAL    NOT NULL CHECK (TimeSeconds >= 0),"
	"  Moves       INTEGER NOT NULL CHECK (Moves >= 0),"
	"  PlayedAt    TEXT    NOT NULL DEFAULT (datetime('now', 'localtime'))"
	");"
	"CREATE INDEX IF NOT EXISTS IX_Results_MazeSize_Time ON Results (MazeSize, TimeSeconds, Moves);";

// Copies a text column into a fixed size buffer, always ending with '\0'
static void CopyTextColumn(sqlite3_stmt* statement, int column, char* destination, size_t destinationSize) {
	const unsigned char* text = sqlite3_column_text(statement, column);

	if (text == nullptr) {
		destination[0] = '\0';
		return;
	}

	strncpy_s(destination, destinationSize, reinterpret_cast<const char*>(text), _TRUNCATE);
}

bool OpenDatabase(const char* filePath) {
	CloseDatabase();

	// Creates the folder of the database file if it doesn't exist yet
	std::error_code error;
	std::filesystem::path folder = std::filesystem::path(filePath).parent_path();

	if (!folder.empty()) {
		std::filesystem::create_directories(folder, error);
	}

	if (sqlite3_open(filePath, &database) != SQLITE_OK) {
		CloseDatabase();
		return false;
	}

	if (sqlite3_exec(database, CREATE_TABLES_SQL, nullptr, nullptr, nullptr) != SQLITE_OK) {
		CloseDatabase();
		return false;
	}

	return true;
}

void CloseDatabase() {
	if (database != nullptr) {
		sqlite3_close(database);
		database = nullptr;
	}
}

bool IsDatabaseOpen() {
	return database != nullptr;
}

bool InsertResult(const ResultRecord& result) {
	if (database == nullptr) {
		return false;
	}

	// Prepared statements with parameters (?) protect against SQL injection
	const char* sql = "INSERT INTO Results (PlayerName, MazeSize, TimeSeconds, Moves) VALUES (?, ?, ?, ?);";
	sqlite3_stmt* statement = nullptr;

	if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK) {
		return false;
	}

	sqlite3_bind_text(statement, 1, result.playerName, -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(statement, 2, result.mazeSize);
	sqlite3_bind_double(statement, 3, result.timeSeconds);
	sqlite3_bind_int(statement, 4, result.moves);

	bool isSaved = sqlite3_step(statement) == SQLITE_DONE;
	sqlite3_finalize(statement);

	return isSaved;
}

int SelectTopResults(int mazeSize, ResultRecord* results, int maxCount) {
	if (database == nullptr || results == nullptr || maxCount <= 0) {
		return -1;
	}

	const char* sql =
		"SELECT PlayerName, MazeSize, TimeSeconds, Moves, PlayedAt FROM Results "
		"WHERE MazeSize = ? ORDER BY TimeSeconds ASC, Moves ASC LIMIT ?;";
	sqlite3_stmt* statement = nullptr;

	if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK) {
		return -1;
	}

	sqlite3_bind_int(statement, 1, mazeSize);
	sqlite3_bind_int(statement, 2, maxCount);

	int count = 0;

	while (count < maxCount && sqlite3_step(statement) == SQLITE_ROW) {
		ResultRecord& record = results[count];

		CopyTextColumn(statement, 0, record.playerName, sizeof(record.playerName));
		record.mazeSize = sqlite3_column_int(statement, 1);
		record.timeSeconds = static_cast<float>(sqlite3_column_double(statement, 2));
		record.moves = sqlite3_column_int(statement, 3);
		CopyTextColumn(statement, 4, record.playedAt, sizeof(record.playedAt));

		count++;
	}

	sqlite3_finalize(statement);
	return count;
}

int SelectRankForTime(int mazeSize, float timeSeconds) {
	if (database == nullptr) {
		return -1;
	}

	const char* sql = "SELECT COUNT(*) FROM Results WHERE MazeSize = ? AND TimeSeconds < ?;";
	sqlite3_stmt* statement = nullptr;

	if (sqlite3_prepare_v2(database, sql, -1, &statement, nullptr) != SQLITE_OK) {
		return -1;
	}

	sqlite3_bind_int(statement, 1, mazeSize);
	sqlite3_bind_double(statement, 2, timeSeconds);

	int rank = -1;

	if (sqlite3_step(statement) == SQLITE_ROW) {
		rank = sqlite3_column_int(statement, 0) + 1;
	}

	sqlite3_finalize(statement);
	return rank;
}
