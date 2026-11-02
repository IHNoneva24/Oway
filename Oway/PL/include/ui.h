#pragma once
#include "raylib.h"

// Shared colors of the interface
const Color COLOR_BACKGROUND = { 18, 20, 28, 255 };
const Color COLOR_BUTTON = { 40, 70, 120, 230 };
const Color COLOR_BUTTON_HOVER = { 60, 100, 170, 245 };
const Color COLOR_WALL = { 220, 220, 230, 255 };
const Color COLOR_EXIT = { 60, 180, 90, 255 };
const Color COLOR_PLAYER = { 240, 180, 40, 255 };

// Draws a button and returns true when it is clicked
bool DrawButton(Rectangle rect, const char* text);

// Draws text centered horizontally on the screen
void DrawCenteredText(const char* text, int y, int fontSize, Color color);
