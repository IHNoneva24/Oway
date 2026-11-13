#include "screen.h"
#include "screens.h"
#include "raylib.h"

int screenW;
int screenH;

GameMode gameMode = SINGLE_PLAYER;
bool isLanGame = false;

// Screen size
void InitScreens(int width, int height) {
	screenW = width;
	screenH = height;
}

// Draw screen
void UpdateScreen(Screen& currentScreen) {
	screenW = GetScreenWidth();
	screenH = GetScreenHeight();

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
}
