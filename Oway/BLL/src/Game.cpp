#include "Game.h"

static GameSession currentSession;

bool StartGameSession(GameSession& session, int mazeSize, unsigned int seed) {
	if (!CreateMaze(session.maze, mazeSize, mazeSize)) {
		return false;
	}

	GenerateMaze(session.maze, seed);

	session.player = Player{};
	session.elapsedSeconds = 0.0f;
	session.isFinished = false;

	return true;
}

void EndGameSession(GameSession& session) {
	DestroyMaze(session.maze);
	session.isFinished = false;
}

bool TryMovePlayer(GameSession& session, Direction direction) {
	if (session.isFinished || HasWall(session.maze, session.player.x, session.player.y, direction)) {
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

bool HasReachedExit(const GameSession& session) {
	return session.player.x == GetExitX(session.maze) && session.player.y == GetExitY(session.maze);
}

void UpdateGameTime(GameSession& session, float deltaSeconds) {
	if (!session.isFinished) {
		session.elapsedSeconds += deltaSeconds;
	}
}

GameSession& GetCurrentSession() {
	return currentSession;
}
