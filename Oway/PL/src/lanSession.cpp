#include "lanSession.h"
#include "ui.h"
#include "LanGame.h"
#include "RaceGame.h"
#include "TeamGame.h"
#include <ctime>

extern int screenW;
extern int screenH;
extern GameMode gameMode;
extern bool isLanGame;

static bool isLeaveMenuOpen = false;
static const char* notice = ""; // shown when the connection ended

static void EndLanGame(Screen& currentScreen) {
	LeaveLanGame();
	isLanGame = false;
	isLeaveMenuOpen = false;
	notice = "";

	if (gameMode == TEAM_PLAY) {
		EndTeamSession(GetCurrentTeamSession());
	}
	else {
		EndRaceSession(GetCurrentRaceSession());
	}

	currentScreen = MAINMENU;
}

bool StartLanSession(const LobbySettings& settings, Screen& currentScreen) {
	bool isStarted = false;
	gameMode = static_cast<GameMode>(settings.mode);

	if (gameMode == TEAM_PLAY) {
		TeamSession& session = GetCurrentTeamSession();
		isStarted = StartTeamSession(session, settings.mazeSize, settings.seed);

		// This computer moves its own hero, the other hero comes from the network
		SetRemoteHero(session, 1 - GetLocalPlayer());
		currentScreen = TEAMGAME;
	}
	else {
		isStarted = StartRaceSession(GetCurrentRaceSession(), settings.mazeSize, settings.seed);
		currentScreen = RACEGAME;
	}

	ResetLanGame();
	isLeaveMenuOpen = false;
	notice = "";
	return isStarted;
}

bool HostLanRematch(int mazeSize, Screen& currentScreen) {
	LobbySettings settings;
	settings.mode = gameMode;
	settings.mazeSize = mazeSize;
	settings.seed = static_cast<unsigned int>(time(nullptr));

	return StartLanRematch(settings) && StartLanSession(settings, currentScreen);
}

void UpdateLanSession(Screen& currentScreen) {
	if (notice[0] != '\0') {
		return;
	}

	LobbySettings settings;

	switch (UpdateLanGame(settings)) {
	case LAN_REMATCH:
		StartLanSession(settings, currentScreen);
		break;

	case LAN_OTHER_LEFT:
		notice = "The other player left the game";
		break;

	case LAN_CONNECTION_LOST:
		notice = "The connection to the other player was lost";
		break;

	case LAN_NOTHING:
		break;
	}
}

void ToggleLanLeaveMenu() {
	isLeaveMenuOpen = !isLeaveMenuOpen;
}

bool DrawLanLeaveMenu(Screen& currentScreen) {
	if (!isLeaveMenuOpen) {
		return false;
	}

	DrawRectangle(0, 0, screenW, screenH, { 0, 0, 0, 170 });

	float y = screenH * 0.3f;
	DrawCenteredText("LEAVE THE GAME?", y, Scaled(70), COLOR_ACCENT);
	DrawCenteredText("The game is not paused - the other player keeps playing", y + Scaled(90), Scaled(26), COLOR_TEXT_DIM);

	y += Scaled(170);

	if (DrawCenteredButton(y, "Keep playing")) {
		isLeaveMenuOpen = false;
	}

	if (DrawCenteredButton(y + Scaled(95), "Leave")) {
		EndLanGame(currentScreen);
	}

	return true;
}

bool DrawLanNotice(Screen& currentScreen) {
	if (notice[0] == '\0') {
		return false;
	}

	DrawRectangle(0, 0, screenW, screenH, { 0, 0, 0, 190 });
	DrawCenteredText(notice, screenH * 0.38f, Scaled(48), COLOR_ERROR);

	if (DrawCenteredButton(screenH * 0.38f + Scaled(110), "Main menu") || IsKeyPressed(KEY_ENTER)) {
		EndLanGame(currentScreen);
	}

	return true;
}
