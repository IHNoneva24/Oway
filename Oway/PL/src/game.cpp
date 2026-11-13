#include "screens.h"
#include "ui.h"
#include "assets.h"
#include "mazeView.h"
#include "Game.h"
#include <ctime>

// from screen.cpp
extern int screenW;
extern int screenH;

// Movement keys
static void HandleMovement(GameSession& session) {
	if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressedRepeat(KEY_W))       TryMovePlayer(session, DIRECTION_UP);
	if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressedRepeat(KEY_D)) TryMovePlayer(session, DIRECTION_RIGHT);
	if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressedRepeat(KEY_S))   TryMovePlayer(session, DIRECTION_DOWN);
	if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT) || IsKeyPressed(KEY_A) || IsKeyPressedRepeat(KEY_A))   TryMovePlayer(session, DIRECTION_LEFT);
}

// Draw maze
static void DrawMaze(const GameSession& session) {
	DrawMazeWalls(session.maze);
	DrawExit(session.maze, true);

	// idle animation
	int frameRow = static_cast<int>(GetTime() * 2) % 2 == 0 ? 0 : 2;
	Rectangle playerTile = GetCellRect(session.maze, session.player.x, session.player.y);
	DrawTile(GetAssets().characters, 0, frameRow, playerTile, session.player.facing == DIRECTION_LEFT);
}

// Draw HUD
static void DrawHud(const GameSession& session) {
	float height = Scaled(HUD_HEIGHT);
	int fontSize = static_cast<int>(Scaled(30));
	int textY = static_cast<int>(height / 2 - fontSize / 2.0f);

	DrawRectangle(0, 0, screenW, static_cast<int>(height), COLOR_PANEL);

	DrawText(TextFormat("Maze: %d x %d", session.maze.width, session.maze.height), static_cast<int>(Scaled(30)), textY, fontSize, RAYWHITE);
	DrawText(TextFormat("Moves: %d", session.player.moves), static_cast<int>(Scaled(330)), textY, fontSize, RAYWHITE);
	DrawText(TextFormat("Time: %.1f s", session.elapsedSeconds), static_cast<int>(Scaled(580)), textY, fontSize, RAYWHITE);

	const char* hint = "ESC - pause";
	DrawText(hint, screenW - MeasureText(hint, fontSize) - static_cast<int>(Scaled(30)), textY, fontSize, COLOR_TEXT_DIM);
}

// Pause menu
static void DrawPauseMenu(GameSession& session, Screen& currentScreen) {
	DrawRectangle(0, 0, screenW, screenH, { 0, 0, 0, 170 });

	float y = screenH * 0.3f;
	DrawCenteredText("PAUSED", y, Scaled(80), COLOR_ACCENT);

	y += Scaled(140);

	if (DrawCenteredButton(y, "Resume")) {
		SetPaused(session, false);
	}

	if (DrawCenteredButton(y + Scaled(95), "Restart (new maze)")) {
		RestartGameSession(session, static_cast<unsigned int>(time(nullptr)));
	}

	if (DrawCenteredButton(y + Scaled(190), "Main menu")) {
		EndGameSession(session);
		currentScreen = MAINMENU;
	}
}

// Game screen
void DrawGame(Screen& currentScreen) {
	GameSession& session = GetCurrentSession();

	if (IsKeyPressed(KEY_ESCAPE)) {
		SetPaused(session, !session.isPaused);
	}

	HandleMovement(session);
	UpdateGameTime(session, GetFrameTime());

	ClearBackground(COLOR_BACKGROUND);
	DrawMaze(session);
	DrawHud(session);

	if (session.isPaused) {
		DrawPauseMenu(session, currentScreen);
	}

	if (session.isFinished) {
		currentScreen = WIN;
	}
}
