#include "screen.h"
#include "screens.h"
#include "raylib.h"

int screenW;
int screenH;

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
	case HELP:        DrawHelp(currentScreen);        break;
	case LEADERBOARD: DrawLeaderboard(currentScreen); break;
	case EXITGAME:   break;
	}
}
