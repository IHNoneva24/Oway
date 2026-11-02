#pragma once
#include "Maze.h"

// Game session logic (player, movement, timer, win)

struct Player {
	int x = 0;
	int y = 0;
	int moves = 0;
};

struct GameSession {
	Maze maze;
	Player player;
	float elapsedSeconds = 0.0f;
	bool isFinished = false;
};

// Creates and generates a new maze, places the player at 0,0
bool StartGameSession(GameSession& session, int mazeSize, unsigned int seed);

void EndGameSession(GameSession& session);

// Moves the player one cell if there is no wall. Returns true if the player moved
bool TryMovePlayer(GameSession& session, Direction direction);

bool HasReachedExit(const GameSession& session);

// Adds the frame time to the timer while the game is running
void UpdateGameTime(GameSession& session, float deltaSeconds);

// The single session shared between the screens
GameSession& GetCurrentSession();
