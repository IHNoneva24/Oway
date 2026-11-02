#include "screens.h"
#include "ui.h"
#include "Game.h"
#include <ctime>

extern int screenW;
extern int screenH;

static int selectedSize = MIN_MAZE_SIZE;

void DrawSizeSelect(Screen& currentScreen) {
	ClearBackground(COLOR_BACKGROUND);

	DrawCenteredText("Choose maze size", 120, 40, RAYWHITE);
	DrawCenteredText(TextFormat("%d x %d", selectedSize, selectedSize), 230, 60, COLOR_PLAYER);
	DrawCenteredText(TextFormat("From %d to %d", MIN_MAZE_SIZE, MAX_MAZE_SIZE), 300, 20, GRAY);

	float centerX = screenW / 2.0f;

	// The buttons and keys never leave the allowed range
	if ((DrawButton({ centerX - 200, 235, 60, 55 }, "-") || IsKeyPressedRepeat(KEY_LEFT) || IsKeyPressed(KEY_LEFT))
		&& selectedSize > MIN_MAZE_SIZE) {
		selectedSize--;
	}

	if ((DrawButton({ centerX + 140, 235, 60, 55 }, "+") || IsKeyPressedRepeat(KEY_RIGHT) || IsKeyPressed(KEY_RIGHT))
		&& selectedSize < MAX_MAZE_SIZE) {
		selectedSize++;
	}

	if (DrawButton({ centerX - 120, 360, 240, 55 }, "Start") || IsKeyPressed(KEY_ENTER)) {
		// A new random maze every time
		if (StartGameSession(GetCurrentSession(), selectedSize, static_cast<unsigned int>(time(nullptr)))) {
			currentScreen = GAME;
		}
	}

	if (DrawButton({ centerX - 120, 435, 240, 55 }, "Back") || IsKeyPressed(KEY_ESCAPE)) {
		currentScreen = MAINMENU;
	}
}
