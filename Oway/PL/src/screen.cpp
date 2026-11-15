#include "screen.h"
#include "screens.h"
#include "raylib.h"

int screenW;
int screenH;

GameMode gameMode = SINGLE_PLAYER;
bool isLanGame = false;

static const float FADE_TIME = 0.16f;
static Screen lastScreen = MAINMENU;
static float fadeTimer = 0.0f;

// Screen size
void InitScreens(int width, int height) {
	screenW = width;
	screenH = height;
}

// Draw screen
void UpdateScreen(Screen& currentScreen) {
	screenW = GetScreenWidth();
	screenH = GetScreenHeight();

	// short fade on screen change
	if (currentScreen != lastScreen) {
		lastScreen = currentScreen;
		fadeTimer = FADE_TIME;
	}

	switch (currentScreen) {
	case MAINMENU:   DrawMainMenu(currentScreen);   break;
	case SIZESELECT: DrawSizeSelect(currentScreen); break;
	case GAME:       DrawGame(currentScreen);       break;
	case WIN:        DrawWin(currentScreen);        break;
	case TEAMGAME:   DrawTeamGame(currentScreen);   break;
	case TEAMWIN:    DrawTeamWin(currentScreen);    break;
	case MODESELECT: DrawModeSelect(currentScreen); break;
	case LOBBY:      DrawLobby(currentScreen);      break;
	case RACEGAME:   DrawRaceGame(currentScreen);   break;
	case RACEWIN:    DrawRaceWin(currentScreen);    break;
	case HELP:        DrawHelp(currentScreen);        break;
	case LEADERBOARD: DrawLeaderboard(currentScreen); break;
	case EXITGAME:   break;
	}

	if (fadeTimer > 0) {
		DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.8f * fadeTimer / FADE_TIME));
		fadeTimer -= GetFrameTime();
	}
}
