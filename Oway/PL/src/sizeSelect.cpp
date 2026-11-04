#include "screens.h"
#include "ui.h"
#include "Game.h"
#include <ctime>

extern int screenW;
extern int screenH;

static const int MAX_INPUT_LENGTH = 2;

static char sizeInput[MAX_INPUT_LENGTH + 1] = "10";
static bool showError = false;

// Changes the size with the - and + buttons, always staying in range
static void ChangeSize(int delta) {
	int size = MIN_MAZE_SIZE;

	if (TryParseMazeSize(sizeInput, size)) {
		size += delta;
	}

	if (size < MIN_MAZE_SIZE) size = MIN_MAZE_SIZE;
	if (size > MAX_MAZE_SIZE) size = MAX_MAZE_SIZE;

	TextCopy(sizeInput, TextFormat("%d", size));
	showError = false;
}

void DrawSizeSelect(Screen& currentScreen) {
	DrawDungeonBackground();

	if (UpdateTextInput(sizeInput, MAX_INPUT_LENGTH, true)) {
		showError = false;
	}

	float centerX = screenW / 2.0f;
	float y = screenH * 0.2f;

	DrawCenteredText("Choose maze size", y, Scaled(70), COLOR_ACCENT);
	DrawCenteredText(TextFormat("Type a number from %d to %d", MIN_MAZE_SIZE, MAX_MAZE_SIZE),
		y + Scaled(95), Scaled(30), COLOR_TEXT_DIM);

	float boxWidth = Scaled(220);
	float boxHeight = Scaled(100);
	float boxY = y + Scaled(170);
	Rectangle box = { centerX - boxWidth / 2, boxY, boxWidth, boxHeight };

	DrawTextBox(box, sizeInput, Scaled(64), showError);

	float smallButton = Scaled(100);

	if (DrawButton({ box.x - smallButton - Scaled(30), boxY, smallButton, boxHeight }, "-")
		|| IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) {
		ChangeSize(-1);
	}

	if (DrawButton({ box.x + boxWidth + Scaled(30), boxY, smallButton, boxHeight }, "+")
		|| IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
		ChangeSize(1);
	}

	if (showError) {
		DrawCenteredText(TextFormat("Please enter a size between %d and %d", MIN_MAZE_SIZE, MAX_MAZE_SIZE),
			boxY + boxHeight + Scaled(20), Scaled(28), COLOR_ERROR);
	}

	float buttonsY = boxY + boxHeight + Scaled(90);

	if (DrawCenteredButton(buttonsY, "Start") || IsKeyPressed(KEY_ENTER)) {
		int size = 0;

		// A new random maze every time
		if (TryParseMazeSize(sizeInput, size)
			&& StartGameSession(GetCurrentSession(), size, static_cast<unsigned int>(time(nullptr)))) {
			currentScreen = GAME;
		}
		else {
			showError = true;
		}
	}

	if (DrawCenteredButton(buttonsY + Scaled(95), "Back") || IsKeyPressed(KEY_ESCAPE)) {
		showError = false;
		currentScreen = MAINMENU;
	}
}
