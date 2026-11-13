#include "screens.h"
#include "ui.h"
#include "Leaderboard.h"

// from screen.cpp
extern int screenW;
extern int screenH;

static int selectedSize = MIN_MAZE_SIZE;

static ResultRecord results[LEADERBOARD_SIZE];
static int resultCount = 0;
static int loadedSize = -1;

// Change size
static void ChangeSelectedSize(int delta) {
	selectedSize += delta;

	if (selectedSize < MIN_MAZE_SIZE) selectedSize = MAX_MAZE_SIZE;
	if (selectedSize > MAX_MAZE_SIZE) selectedSize = MIN_MAZE_SIZE;
}

// Table row
static void DrawRow(float x, float y, float width, const char* rank, const char* name,
	const char* time, const char* moves, const char* date, Color color) {
	int fontSize = static_cast<int>(Scaled(28));

	DrawText(rank, static_cast<int>(x), static_cast<int>(y), fontSize, color);
	DrawText(name, static_cast<int>(x + width * 0.08f), static_cast<int>(y), fontSize, color);
	DrawText(time, static_cast<int>(x + width * 0.42f), static_cast<int>(y), fontSize, color);
	DrawText(moves, static_cast<int>(x + width * 0.58f), static_cast<int>(y), fontSize, color);
	DrawText(date, static_cast<int>(x + width * 0.72f), static_cast<int>(y), fontSize, color);
}

// Leaderboard screen
void DrawLeaderboard(Screen& currentScreen) {
	DrawDungeonBackground();

	float panelWidth = Scaled(1200);
	float panelHeight = Scaled(940);
	float panelX = screenW / 2.0f - panelWidth / 2;
	float panelY = screenH / 2.0f - panelHeight / 2;
	DrawPanel({ panelX, panelY, panelWidth, panelHeight });

	float y = panelY + Scaled(35);
	DrawCenteredText("LEADERBOARD", y, Scaled(64), COLOR_ACCENT);

	y += Scaled(100);
	float arrowSize = Scaled(70);

	if (DrawButton({ screenW / 2.0f - Scaled(220), y, arrowSize, arrowSize }, "<")
		|| IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
		ChangeSelectedSize(-1);
	}

	if (DrawButton({ screenW / 2.0f + Scaled(150), y, arrowSize, arrowSize }, ">")
		|| IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
		ChangeSelectedSize(1);
	}

	DrawCenteredText(TextFormat("%d x %d", selectedSize, selectedSize), y + Scaled(12), Scaled(46), RAYWHITE);

	y += Scaled(110);
	float tableX = panelX + Scaled(60);
	float tableWidth = panelWidth - Scaled(120);

	DrawRow(tableX, y, tableWidth, "#", "Name", "Time", "Moves", "Date", COLOR_TEXT_DIM);
	DrawRectangle(static_cast<int>(tableX), static_cast<int>(y + Scaled(40)), static_cast<int>(tableWidth),
		static_cast<int>(Scaled(3)), { 92, 60, 52, 255 });

	y += Scaled(60);

	if (!IsLeaderboardAvailable()) {
		DrawCenteredText("Database could not be opened", y + Scaled(40), Scaled(30), COLOR_ERROR);
	}
	else {
		if (loadedSize != selectedSize) {
			resultCount = GetLeaderboard(selectedSize, results);
			loadedSize = selectedSize;
		}

		if (resultCount == 0) {
			DrawCenteredText("No results yet - be the first!", y + Scaled(40), Scaled(30), COLOR_TEXT_DIM);
		}

		for (int i = 0; i < resultCount; i++) {
			Color color = i == 0 ? COLOR_ACCENT : RAYWHITE;

			DrawRow(tableX, y + i * Scaled(46), tableWidth, TextFormat("%d", i + 1), results[i].playerName,
				TextFormat("%.1f s", results[i].timeSeconds), TextFormat("%d", results[i].moves),
				results[i].playedAt, color);
		}
	}

	if (DrawCenteredButton(panelY + panelHeight - Scaled(100), "Back") || IsKeyPressed(KEY_ESCAPE)) {
		loadedSize = -1;
		currentScreen = MAINMENU;
	}
}
