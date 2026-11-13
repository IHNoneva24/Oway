#include "screens.h"
#include "ui.h"

// from screen.cpp
extern int screenW;
extern int screenH;

// Main menu
void DrawMainMenu(Screen& currentScreen) {
	DrawDungeonBackground();

	float titleSize = Scaled(140);
	float titleY = screenH * 0.15f;
	int titleWidth = MeasureText("OWAY", static_cast<int>(titleSize));

	DrawCenteredText("OWAY", titleY + Scaled(6), titleSize, BLACK);
	DrawCenteredText("OWAY", titleY, titleSize, COLOR_ACCENT);
	DrawCenteredText("Find the way out of the dungeon", titleY + titleSize + Scaled(10), Scaled(30), COLOR_TEXT_DIM);

	float torchSize = Scaled(96);
	DrawTorch(screenW / 2.0f - titleWidth / 2.0f - torchSize - Scaled(30), titleY + Scaled(10), torchSize);
	DrawTorch(screenW / 2.0f + titleWidth / 2.0f + Scaled(30), titleY + Scaled(10), torchSize);

	float y = screenH * 0.45f;
	float step = Scaled(95);

	if (DrawCenteredButton(y, "Play")) {
		currentScreen = MODESELECT;
	}

	if (DrawCenteredButton(y + step, "Leaderboard")) {
		currentScreen = LEADERBOARD;
	}

	if (DrawCenteredButton(y + step * 2, "How to play")) {
		currentScreen = HELP;
	}

	if (DrawCenteredButton(y + step * 3, "Exit")) {
		currentScreen = EXITGAME;
	}

	DrawText("F11 - fullscreen / window", static_cast<int>(Scaled(20)), screenH - static_cast<int>(Scaled(40)),
		static_cast<int>(Scaled(22)), COLOR_TEXT_DIM);
}
