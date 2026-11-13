#include "tests.h"
#include "testFramework.h"
#include "Game.h"

// Wall follower
static bool SolveWithWallFollower(GameSession& session, int maxSteps) {
	Direction heading = DIRECTION_RIGHT;

	for (int step = 0; step < maxSteps && !session.isFinished; step++) {
		Direction order[4] = {
			static_cast<Direction>((heading + 1) % 4), heading,
			static_cast<Direction>((heading + 3) % 4), static_cast<Direction>((heading + 2) % 4)
		};

		for (Direction direction : order) {
			if (TryMovePlayer(session, direction)) {
				heading = direction;
				break;
			}
		}
	}

	return session.isFinished;
}

// Starting
static void TestStart() {
	GameSession session;

	Check(!StartGameSession(session, 8, 1u), "a game with size 8 doesn't start");
	Check(StartGameSession(session, 15, 1u), "a game with size 15 starts");
	Check(session.player.x == 0 && session.player.y == 0, "the player starts at 0,0 (top left)");
	CheckEqual(0, session.player.moves, "the move counter starts at 0");
	Check(!session.isFinished && !session.isPaused, "a new game is not finished and not paused");

	EndGameSession(session);
	Check(session.maze.cells == nullptr, "EndGameSession frees the maze");
}

// Movement
static void TestMovement() {
	GameSession session;
	StartGameSession(session, 15, 3u);

	Check(!TryMovePlayer(session, DIRECTION_UP), "the player can't go up through the outer wall");
	Check(!TryMovePlayer(session, DIRECTION_LEFT), "the player can't go left through the outer wall");
	CheckEqual(0, session.player.moves, "a blocked move is not counted");

	Direction open = HasWall(session.maze, 0, 0, DIRECTION_RIGHT) ? DIRECTION_DOWN : DIRECTION_RIGHT;
	Check(TryMovePlayer(session, open), "the player can walk through an opening");
	CheckEqual(1, session.player.moves, "a move is counted");
	Check(session.player.facing == open, "the player faces the direction of the last move");

	bool isPositionRight = open == DIRECTION_RIGHT
		? session.player.x == 1 && session.player.y == 0
		: session.player.x == 0 && session.player.y == 1;
	Check(isPositionRight, "the player is on the next cell after the move");

	EndGameSession(session);
}

// Pause, timer
static void TestPauseAndTimer() {
	GameSession session;
	StartGameSession(session, 10, 5u);

	UpdateGameTime(session, 1.5f);
	Check(session.elapsedSeconds > 1.49f && session.elapsedSeconds < 1.51f, "the timer counts the time");

	SetPaused(session, true);
	Check(session.isPaused, "the game can be paused");
	UpdateGameTime(session, 10.0f);
	Check(session.elapsedSeconds < 1.51f, "the timer stops while paused");
	Check(!TryMovePlayer(session, DIRECTION_RIGHT) && !TryMovePlayer(session, DIRECTION_DOWN),
		"the player can't move while paused");

	SetPaused(session, false);
	Check(!session.isPaused, "the game can be resumed");

	EndGameSession(session);
}

// Winning
static void TestWinning() {
	GameSession session;
	StartGameSession(session, 20, 11u);

	Check(SolveWithWallFollower(session, 10000), "the exit can be reached (the maze is solvable)");
	Check(HasReachedExit(session), "the player is on the exit at the end");
	Check(session.player.x == 19 && session.player.y == 19, "the exit is the bottom right cell");

	float timeAtWin = session.elapsedSeconds;
	UpdateGameTime(session, 5.0f);
	Check(session.elapsedSeconds == timeAtWin, "the timer stops when the game is won");
	Check(!TryMovePlayer(session, DIRECTION_LEFT), "the player can't move after winning");

	SetPaused(session, true);
	Check(!session.isPaused, "a finished game can't be paused");

	Check(RestartGameSession(session, 12u), "the game can be restarted");
	Check(session.maze.width == 20 && !session.isFinished && session.player.moves == 0,
		"a restart keeps the size and starts from the beginning");

	EndGameSession(session);
}

// Game tests
void RunGameTests() {
	BeginTestGroup("Single player game (BLL/Game)");

	TestStart();
	TestMovement();
	TestPauseAndTimer();
	TestWinning();
}
