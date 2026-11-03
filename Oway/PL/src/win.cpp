#include "screens.h"
#include "ui.h"
#include "Game.h"
#include <ctime>

extern int screenW;
extern int screenH;

void DrawWin(Screen& currentScreen) {
	GameSession& session = GetCurrentSession();

	DrawDungeonBackground();

	float panelWidth = Scaled(800);
	float panelHeight = Scaled(720);
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

	y += Scaled(200);

	if (DrawCenteredButton(y, "Play again") || IsKeyPressed(KEY_ENTER)) {
		RestartGameSession(session, static_cast<unsigned int>(time(nullptr)));
		currentScreen = GAME;
	}

	if (DrawCenteredButton(y + Scaled(95), "Change size")) {
		EndGameSession(session);
		currentScreen = SIZESELECT;
	}

	if (DrawCenteredButton(y + Scaled(190), "Main menu") || IsKeyPressed(KEY_ESCAPE)) {
		EndGameSession(session);
		currentScreen = MAINMENU;
	}
}
