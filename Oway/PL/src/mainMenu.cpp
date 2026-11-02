#include "screens.h"
#include "ui.h"

extern int screenW;
extern int screenH;

void DrawMainMenu(Screen& currentScreen) {
	ClearBackground(COLOR_BACKGROUND);

	DrawCenteredText("OWAY", 120, 80, RAYWHITE);
	DrawCenteredText("Find the way out of the maze", 210, 22, GRAY);

	float buttonX = screenW / 2.0f - 120;

	if (DrawButton({ buttonX, 300, 240, 55 }, "Single player")) {
		currentScreen = SIZESELECT;
	}

	if (DrawButton({ buttonX, 375, 240, 55 }, "Exit")) {
		currentScreen = EXITGAME;
	}
}
