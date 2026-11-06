#include "screen.h"
#include "screens.h"
#include "raylib.h"

int screenW;
int screenH;

// Selected in the main menu, used by the size select screen to start the right game
GameMode gameMode = SINGLE_PLAYER;
bool isLanGame = false; // true when the multiplayer game is played over the network

void InitScreens(int width, int height) {
	screenW = width;
	screenH = height;
}

void UpdateScreen(Screen& currentScreen) {
	// The window can be resized at any time, so the size is read every frame
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
