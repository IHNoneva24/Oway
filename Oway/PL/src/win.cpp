#include "screens.h"
#include "ui.h"
#include "Game.h"
#include "Leaderboard.h"
#include <ctime>

// from screen.cpp
extern int screenW;
extern int screenH;

static char playerName[MAX_PLAYER_NAME_LENGTH + 1] = "";
static bool isSaved = false;
static int savedRank = 0;
static SaveResultStatus lastStatus = SAVE_OK;
static bool hasTriedToSave = false;

// Reset saving
static void LeaveWinScreen() {
	isSaved = false;
	hasTriedToSave = false;
}

// Save result
static void TrySaveResult(const GameSession& session) {
	hasTriedToSave = true;
	lastStatus = SaveGameResult(session, playerName, savedRank);
	isSaved = lastStatus == SAVE_OK;
}

// Save message
static const char* GetSaveMessage() {
	switch (lastStatus) {
	case SAVE_OK:             return "Saved!";
	case SAVE_INVALID_NAME:   return "Name: 1-16 letters, digits, spaces, _ or -";
	case SAVE_NOT_FINISHED:   return "The game is not finished";
	case SAVE_DATABASE_ERROR: return "Could not save - database error";
	}

	return "";
}

// Name input
static void DrawSaveSection(const GameSession& session, float y) {
	if (!IsLeaderboardAvailable()) {
		DrawCenteredText("Leaderboard unavailable (database could not be opened)", y + Scaled(30), Scaled(26), COLOR_ERROR);
		return;
	}

	if (isSaved) {
		DrawCenteredText(TextFormat("Saved! You are #%d on %dx%d", savedRank, session.maze.width, session.maze.height),
			y + Scaled(30), Scaled(34), COLOR_ACCENT);
		return;
	}

	if (UpdateTextInput(playerName, MAX_PLAYER_NAME_LENGTH, false)) {
		hasTriedToSave = false;
	}

	float boxWidth = Scaled(420);
	float buttonWidth = Scaled(160);
	float height = Scaled(70);
	float startX = screenW / 2.0f - (boxWidth + Scaled(20) + buttonWidth) / 2;
	bool hasError = hasTriedToSave && !isSaved;

	DrawText("Your name:", static_cast<int>(startX), static_cast<int>(y - Scaled(36)), static_cast<int>(Scaled(26)), COLOR_TEXT_DIM);
	DrawTextBox({ startX, y, boxWidth, height }, playerName, Scaled(36), hasError);

	if (DrawButton({ startX + boxWidth + Scaled(20), y, buttonWidth, height }, "Save") || IsKeyPressed(KEY_ENTER)) {
		TrySaveResult(session);
	}

	if (hasError) {
		DrawCenteredText(GetSaveMessage(), y + height + Scaled(12), Scaled(24), COLOR_ERROR);
	}
}

// Win screen
void DrawWin(Screen& currentScreen) {
	GameSession& session = GetCurrentSession();

	DrawDungeonBackground();

	float panelWidth = Scaled(900);
	float panelHeight = Scaled(900);
	float panelY = screenH / 2.0f - panelHeight / 2;
	DrawPanel({ screenW / 2.0f - panelWidth / 2, panelY, panelWidth, panelHeight });

	float torchSize = Scaled(80);
	DrawTorch(screenW / 2.0f - panelWidth / 2 + Scaled(30), panelY + Scaled(30), torchSize);
	DrawTorch(screenW / 2.0f + panelWidth / 2 - Scaled(30) - torchSize, panelY + Scaled(30), torchSize);

	float y = panelY + Scaled(45);
	DrawCenteredText("YOU ESCAPED!", y, Scaled(70), COLOR_ACCENT);

	y += Scaled(130);
	DrawCenteredText(TextFormat("Maze: %d x %d", session.maze.width, session.maze.height), y, Scaled(36), RAYWHITE);
	DrawCenteredText(TextFormat("Time: %.1f seconds", session.elapsedSeconds), y + Scaled(55), Scaled(36), RAYWHITE);
	DrawCenteredText(TextFormat("Moves: %d", session.player.moves), y + Scaled(110), Scaled(36), RAYWHITE);

	DrawSaveSection(session, y + Scaled(220));

	y += Scaled(380);

	if (DrawCenteredButton(y, "Play again")) {
		LeaveWinScreen();
		RestartGameSession(session, static_cast<unsigned int>(time(nullptr)));
		currentScreen = GAME;
	}

	if (DrawCenteredButton(y + Scaled(95), "Change size")) {
		LeaveWinScreen();
		EndGameSession(session);
		currentScreen = SIZESELECT;
	}

	if (DrawCenteredButton(y + Scaled(190), "Main menu") || IsKeyPressed(KEY_ESCAPE)) {
		LeaveWinScreen();
		EndGameSession(session);
		currentScreen = MAINMENU;
	}
}
