#include "screens.h"
#include "ui.h"
#include "TeamGame.h"
#include <ctime>

extern int screenW;
extern int screenH;

void DrawTeamWin(Screen& currentScreen) {
	TeamSession& session = GetCurrentTeamSession();

	DrawDungeonBackground();

	float panelWidth = Scaled(900);
	float panelHeight = Scaled(760);
	float panelY = screenH / 2.0f - panelHeight / 2;
	DrawPanel({ screenW / 2.0f - panelWidth / 2, panelY, panelWidth, panelHeight });

	float torchSize = Scaled(80);
	DrawTorch(screenW / 2.0f - panelWidth / 2 + Scaled(30), panelY + Scaled(30), torchSize);
	DrawTorch(screenW / 2.0f + panelWidth / 2 - Scaled(30) - torchSize, panelY + Scaled(30), torchSize);

	float y = panelY + Scaled(45);
	DrawCenteredText("TEAM ESCAPED!", y, Scaled(70), COLOR_ACCENT);

	y += Scaled(130);
	DrawCenteredText(TextFormat("Maze: %d x %d", session.maze.width, session.maze.height), y, Scaled(36), RAYWHITE);
	DrawCenteredText(TextFormat("Time: %.1f seconds", session.elapsedSeconds), y + Scaled(55), Scaled(36), RAYWHITE);
	DrawCenteredText(TextFormat("Moves: %d (fire %d, water %d)", GetTeamMoves(session),
		session.heroes[HERO_FIRE].moves, session.heroes[HERO_WATER].moves), y + Scaled(110), Scaled(36), RAYWHITE);

	y += Scaled(220);

	if (DrawCenteredButton(y, "Play again")) {
		RestartTeamSession(session, static_cast<unsigned int>(time(nullptr)));
		currentScreen = TEAMGAME;
	}

	if (DrawCenteredButton(y + Scaled(95), "Change size")) {
		EndTeamSession(session);
		currentScreen = SIZESELECT;
	}

	if (DrawCenteredButton(y + Scaled(190), "Main menu") || IsKeyPressed(KEY_ESCAPE)) {
		EndTeamSession(session);
		currentScreen = MAINMENU;
	}
}
