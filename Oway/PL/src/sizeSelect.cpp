#include "screens.h"
#include "ui.h"
#include "Game.h"
#include <ctime>

extern int screenW;
extern int screenH;

static const int MAX_INPUT_LENGTH = 2;

static char sizeInput[MAX_INPUT_LENGTH + 1] = "10";
static int inputLength = 2;
static bool showError = false;

// Lets the user type only digits into the size box
static void HandleTyping() {
	int key = GetCharPressed();

	while (key > 0) {
		if (key >= '0' && key <= '9' && inputLength < MAX_INPUT_LENGTH) {
			sizeInput[inputLength++] = static_cast<char>(key);
			sizeInput[inputLength] = '\0';
			showError = false;
		}

		key = GetCharPressed();
	}

	if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && inputLength > 0) {
		sizeInput[--inputLength] = '\0';
		showError = false;
	}
}

// Changes the size with the - and + buttons, always staying in range
static void ChangeSize(int delta) {
	int size = MIN_MAZE_SIZE;

	if (TryParseMazeSize(sizeInput, size)) {
		size += delta;
	}

	if (size < MIN_MAZE_SIZE) size = MIN_MAZE_SIZE;
	if (size > MAX_MAZE_SIZE) size = MAX_MAZE_SIZE;

	TextCopy(sizeInput, TextFormat("%d", size));
	inputLength = TextLength(sizeInput);
	showError = false;
}

void DrawSizeSelect(Screen& currentScreen) {
	DrawDungeonBackground();
	HandleTyping();

	float centerX = screenW / 2.0f;
	float y = screenH * 0.2f;

	DrawCenteredText("Choose maze size", y, Scaled(70), COLOR_ACCENT);
	DrawCenteredText(TextFormat("Type a number from %d to %d", MIN_MAZE_SIZE, MAX_MAZE_SIZE),
		y + Scaled(95), Scaled(30), COLOR_TEXT_DIM);

	// Input box with a blinking cursor
	float boxWidth = Scaled(220);
	float boxHeight = Scaled(100);
	float boxY = y + Scaled(170);
	Rectangle box = { centerX - boxWidth / 2, boxY, boxWidth, boxHeight };

	DrawPanel(box);
	DrawRectangleLinesEx(box, Scaled(3), showError ? COLOR_ERROR : COLOR_ACCENT);

	const char* shown = (static_cast<int>(GetTime() * 2) % 2 == 0) ? TextFormat("%s_", sizeInput) : sizeInput;
	DrawCenteredText(shown, boxY + Scaled(18), Scaled(64), RAYWHITE);

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
