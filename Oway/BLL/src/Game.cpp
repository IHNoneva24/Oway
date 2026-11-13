#include "Game.h"

static GameSession currentSession;

// Start game
bool StartGameSession(GameSession& session, int mazeSize, unsigned int seed) {
	if (!CreateMaze(session.maze, mazeSize, mazeSize)) {
		return false;
	}

	GenerateMaze(session.maze, seed);

	session.player = Player{};
	session.elapsedSeconds = 0.0f;
	session.isFinished = false;
	session.isPaused = false;

	return true;
}

// Restart game
bool RestartGameSession(GameSession& session, unsigned int seed) {
	return StartGameSession(session, session.maze.width, seed);
}

// End game
void EndGameSession(GameSession& session) {
	DestroyMaze(session.maze);
	session.isFinished = false;
	session.isPaused = false;
}

// Pause game
void SetPaused(GameSession& session, bool isPaused) {
	session.isPaused = isPaused && !session.isFinished;
}

// Move player
bool TryMovePlayer(GameSession& session, Direction direction) {
	if (session.isFinished || session.isPaused) {
		return false;
	}

	session.player.facing = direction;

	if (HasWall(session.maze, session.player.x, session.player.y, direction)) {
		return false;
	}

	switch (direction) {
	case DIRECTION_UP:    session.player.y--; break;
	case DIRECTION_RIGHT: session.player.x++; break;
	case DIRECTION_DOWN:  session.player.y++; break;
	case DIRECTION_LEFT:  session.player.x--; break;
	}

	session.player.moves++;
	session.isFinished = HasReachedExit(session);

	return true;
}

// Exit check
bool HasReachedExit(const GameSession& session) {
	return session.player.x == GetExitX(session.maze) && session.player.y == GetExitY(session.maze);
}

// Update timer
void UpdateGameTime(GameSession& session, float deltaSeconds) {
	if (!session.isFinished && !session.isPaused) {
		session.elapsedSeconds += deltaSeconds;
	}
}

// Current game
GameSession& GetCurrentSession() {
	return currentSession;
}
