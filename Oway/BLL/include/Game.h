#pragma once
#include "Maze.h"

// Player data
struct Player {
	int x = 0;
	int y = 0;
	int moves = 0;
	Direction facing = DIRECTION_RIGHT;
};

// Game state
struct GameSession {
	Maze maze;
	Player player;
	float elapsedSeconds = 0.0f;
	bool isFinished = false;
	bool isPaused = false;
};

// Start game
bool StartGameSession(GameSession& session, int mazeSize, unsigned int seed);

// Restart game
bool RestartGameSession(GameSession& session, unsigned int seed);

// End game
void EndGameSession(GameSession& session);

// Pause game
void SetPaused(GameSession& session, bool isPaused);

// Move player
bool TryMovePlayer(GameSession& session, Direction direction);

// Exit check
bool HasReachedExit(const GameSession& session);

// Update timer
void UpdateGameTime(GameSession& session, float deltaSeconds);

// Current game
GameSession& GetCurrentSession();
