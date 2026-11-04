#pragma once
#include "enum.h"

// Declarations of all screen functions (one .cpp file per screen)
void DrawMainMenu(Screen& currentScreen);
void DrawSizeSelect(Screen& currentScreen);
void DrawGame(Screen& currentScreen);
void DrawWin(Screen& currentScreen);
void DrawHelp(Screen& currentScreen);
void DrawLeaderboard(Screen& currentScreen);
