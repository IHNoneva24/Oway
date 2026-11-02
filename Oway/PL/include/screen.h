#pragma once
#include "enum.h"

void InitScreens(int width, int height);

// Calls the draw function of the current screen
void UpdateScreen(Screen& currentScreen);
