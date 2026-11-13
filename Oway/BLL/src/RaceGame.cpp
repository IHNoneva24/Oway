#include "RaceGame.h"

static RaceSession currentRaceSession;

// Racer check
static bool IsValidRacer(int racer) {
	return racer >= 0 && racer < RACER_COUNT;
}

// Start race
bool StartRaceSession(RaceSession& session, int mazeSize, unsigned int seed) {
	for (int i = 0; i < RACER_COUNT; i++) {
		if (!CreateMaze(session.mazes[i], mazeSize, mazeSize)) {
			EndRaceSession(session);
			return false;
		}

		GenerateMaze(session.mazes[i], seed);

		session.racers[i] = Player{};
		session.hasEscaped[i] = false;
		session.escapeSeconds[i] = 0.0f;
	}

	session.elapsedSeconds = 0.0f;
	session.isFinished = false;
	session.isPaused = false;

	return true;
}

// Restart race
bool RestartRaceSession(RaceSession& session, unsigned int seed) {
	return StartRaceSession(session, session.mazes[0].width, seed);
}

// End race
void EndRaceSession(RaceSession& session) {
	for (int i = 0; i < RACER_COUNT; i++) {
		DestroyMaze(session.mazes[i]);
	}

	session.isFinished = false;
	session.isPaused = false;
}

// Pause race
void SetRacePaused(RaceSession& session, bool isPaused) {
	session.isPaused = isPaused && !session.isFinished;
}

// Move racer
bool TryMoveRacer(RaceSession& session, int racer, Direction direction) {
	if (session.isFinished || session.isPaused || !IsValidRacer(racer) || session.hasEscaped[racer]) {
		return false;
	}

	Player& player = session.racers[racer];
	const Maze& maze = session.mazes[racer];
	player.facing = direction;

	if (HasWall(maze, player.x, player.y, direction)) {
		return false;
	}

	switch (direction) {
	case DIRECTION_UP:    player.y--; break;
	case DIRECTION_RIGHT: player.x++; break;
	case DIRECTION_DOWN:  player.y++; break;
	case DIRECTION_LEFT:  player.x--; break;
	}

	player.moves++;

	if (player.x == GetExitX(maze) && player.y == GetExitY(maze)) {
		session.hasEscaped[racer] = true;
		session.escapeSeconds[racer] = session.elapsedSeconds;
	}

	session.isFinished = session.hasEscaped[0] && session.hasEscaped[1];

	return true;
}

// Remote racer
void ApplyRemoteRacer(RaceSession& session, int racer, int x, int y, int moves, Direction facing,
	bool hasEscaped, float escapeSeconds) {
	if (!IsValidRacer(racer) || !IsInsideMaze(session.mazes[racer], x, y)) {
		return;
	}

	Player& player = session.racers[racer];
	player.x = x;
	player.y = y;
	player.moves = moves;
	player.facing = facing;

	if (hasEscaped && !session.hasEscaped[racer]) {
		session.hasEscaped[racer] = true;
		session.escapeSeconds[racer] = escapeSeconds;
	}

	session.isFinished = session.hasEscaped[0] && session.hasEscaped[1];
}

// Update timer
void UpdateRace(RaceSession& session, float deltaSeconds) {
	if (!session.isFinished && !session.isPaused) {
		session.elapsedSeconds += deltaSeconds;
	}
}

// Race winner
int GetRaceWinner(const RaceSession& session) {
	int winner = -1;

	for (int i = 0; i < RACER_COUNT; i++) {
		if (session.hasEscaped[i] && (winner == -1 || session.escapeSeconds[i] < session.escapeSeconds[winner])) {
			winner = i;
		}
	}

	return winner;
}

// Current race
RaceSession& GetCurrentRaceSession() {
	return currentRaceSession;
}
