#include "screens.h"
#include "ui.h"
#include "RaceGame.h"
#include "Lobby.h"
#include "lanSession.h"
#include <ctime>

extern int screenW;
extern int screenH;
extern bool isLanGame;

// LAN: only the host can start a rematch, the other player waits for it
static void DrawLanButtons(const RaceSession& session, Screen& currentScreen, float y) {
	UpdateLanSession(currentScreen);

	if (DrawLanNotice(currentScreen) || currentScreen != RACEWIN) {
		return;
	}

	if (IsHost()) {
		if (DrawCenteredButton(y, "Rematch")) {
			HostLanRematch(session.mazes[0].width, currentScreen);
		}
	}
	else {
		DrawCenteredText("Waiting for the host to start a rematch...", y + Scaled(20), Scaled(28), COLOR_TEXT_DIM);
	}

	if (DrawCenteredButton(y + Scaled(95), "Leave") || IsKeyPressed(KEY_ESCAPE)) {
		ToggleLanLeaveMenu();
	}

	DrawLanLeaveMenu(currentScreen);
}

void DrawRaceWin(Screen& currentScreen) {
	RaceSession& session = GetCurrentRaceSession();

	DrawDungeonBackground();

	float panelWidth = Scaled(900);
	float panelHeight = Scaled(800);
	float panelY = screenH / 2.0f - panelHeight / 2;
	DrawPanel({ screenW / 2.0f - panelWidth / 2, panelY, panelWidth, panelHeight });

	float torchSize = Scaled(80);
	DrawTorch(screenW / 2.0f - panelWidth / 2 + Scaled(30), panelY + Scaled(30), torchSize);
	DrawTorch(screenW / 2.0f + panelWidth / 2 - Scaled(30) - torchSize, panelY + Scaled(30), torchSize);

	int winner = GetRaceWinner(session);
	float y = panelY + Scaled(45);
	DrawCenteredText(TextFormat("PLAYER %d WINS!", winner + 1), y, Scaled(70), COLOR_ACCENT);

	y += Scaled(130);
	DrawCenteredText(TextFormat("Maze: %d x %d", session.mazes[0].width, session.mazes[0].height), y, Scaled(36), RAYWHITE);

	for (int i = 0; i < RACER_COUNT; i++) {
		DrawCenteredText(TextFormat("Player %d:  %.1f s,  %d moves", i + 1, session.escapeSeconds[i], session.racers[i].moves),
			y + Scaled(60.0f + i * 50.0f), Scaled(34), i == winner ? COLOR_ACCENT : RAYWHITE);
	}

	DrawCenteredText("Race results are not saved to the leaderboard", y + Scaled(175), Scaled(24), COLOR_TEXT_DIM);

	y += Scaled(260);

	if (isLanGame) {
		DrawLanButtons(session, currentScreen, y);
		return;
	}

	if (DrawCenteredButton(y, "Rematch")) {
		RestartRaceSession(session, static_cast<unsigned int>(time(nullptr)));
		currentScreen = RACEGAME;
	}

	if (DrawCenteredButton(y + Scaled(95), "Change size")) {
		EndRaceSession(session);
		currentScreen = SIZESELECT;
	}

	if (DrawCenteredButton(y + Scaled(190), "Main menu") || IsKeyPressed(KEY_ESCAPE)) {
		EndRaceSession(session);
		currentScreen = MAINMENU;
	}
}
