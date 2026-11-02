#include "screen.h"
#include "screens.h"

int screenW;
int screenH;

void InitScreens(int width, int height) {
	screenW = width;
	screenH = height;
}

void UpdateScreen(Screen& currentScreen) {
	switch (currentScreen) {
	case MAINMENU:   DrawMainMenu(currentScreen);   break;
	case SIZESELECT: DrawSizeSelect(currentScreen); break;
	case GAME:       DrawGame(currentScreen);       break;
	case EXITGAME:   break;
	}
}
