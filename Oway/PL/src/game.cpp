#include "screens.h"
#include "ui.h"
#include "Game.h"

extern int screenW;
extern int screenH;

static const int HUD_HEIGHT = 60;
static const int MAZE_MARGIN = 20;

// Reads the arrow keys and moves the player
static void HandleMovement(GameSession& session) {
	if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))       TryMovePlayer(session, DIRECTION_UP);
	if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) TryMovePlayer(session, DIRECTION_RIGHT);
	if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))   TryMovePlayer(session, DIRECTION_DOWN);
	if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT))   TryMovePlayer(session, DIRECTION_LEFT);
}

// Draws the walls, the exit and the player, top-down view
static void DrawMaze(const GameSession& session) {
	const Maze& maze = session.maze;

	int availableWidth = screenW - MAZE_MARGIN * 2;
	int availableHeight = screenH - HUD_HEIGHT - MAZE_MARGIN * 2;
	int cellSize = availableWidth / maze.width < availableHeight / maze.height
		? availableWidth / maze.width : availableHeight / maze.height;

	int offsetX = (screenW - cellSize * maze.width) / 2;
	int offsetY = HUD_HEIGHT + (screenH - HUD_HEIGHT - cellSize * maze.height) / 2;

	DrawRectangle(offsetX + GetExitX(maze) * cellSize, offsetY + GetExitY(maze) * cellSize, cellSize, cellSize, COLOR_EXIT);

	for (int y = 0; y < maze.height; y++) {
		for (int x = 0; x < maze.width; x++) {
			float left = static_cast<float>(offsetX + x * cellSize);
			float top = static_cast<float>(offsetY + y * cellSize);
			float right = left + cellSize;
			float bottom = top + cellSize;

			if (HasWall(maze, x, y, DIRECTION_UP))    DrawLineEx({ left, top }, { right, top }, 2, COLOR_WALL);
			if (HasWall(maze, x, y, DIRECTION_RIGHT)) DrawLineEx({ right, top }, { right, bottom }, 2, COLOR_WALL);
			if (HasWall(maze, x, y, DIRECTION_DOWN))  DrawLineEx({ left, bottom }, { right, bottom }, 2, COLOR_WALL);
			if (HasWall(maze, x, y, DIRECTION_LEFT))  DrawLineEx({ left, top }, { left, bottom }, 2, COLOR_WALL);
		}
	}

	DrawCircle(offsetX + session.player.x * cellSize + cellSize / 2,
		offsetY + session.player.y * cellSize + cellSize / 2, cellSize * 0.35f, COLOR_PLAYER);
}

void DrawGame(Screen& currentScreen) {
	GameSession& session = GetCurrentSession();

	HandleMovement(session);
	UpdateGameTime(session, GetFrameTime());

	ClearBackground(COLOR_BACKGROUND);
	DrawMaze(session);

	DrawText(TextFormat("Moves: %d", session.player.moves), 20, 18, 24, RAYWHITE);
	DrawText(TextFormat("Time: %.1f s", session.elapsedSeconds), 200, 18, 24, RAYWHITE);
	DrawText("ESC - menu", screenW - 150, 18, 20, GRAY);

	if (session.isFinished) {
		DrawCenteredText("You escaped! Press ENTER", screenH / 2 - 20, 40, COLOR_PLAYER);

		if (IsKeyPressed(KEY_ENTER)) {
			EndGameSession(session);
			currentScreen = MAINMENU;
		}
	}

	if (IsKeyPressed(KEY_ESCAPE)) {
		EndGameSession(session);
		currentScreen = MAINMENU;
	}
}
