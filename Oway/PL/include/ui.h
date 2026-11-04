#pragma once
#include "raylib.h"

// Shared colors of the interface
const Color COLOR_BACKGROUND = { 26, 18, 30, 255 };
const Color COLOR_PANEL = { 20, 14, 24, 220 };
const Color COLOR_BUTTON = { 92, 60, 52, 235 };
const Color COLOR_BUTTON_HOVER = { 140, 90, 66, 250 };
const Color COLOR_ACCENT = { 240, 180, 60, 255 };
const Color COLOR_ERROR = { 230, 80, 70, 255 };
const Color COLOR_TEXT_DIM = { 170, 160, 170, 255 };

// Scale of the interface compared to a 1920x1080 screen, so it fits any window size
float GetUiScale();

// Value multiplied by the UI scale
float Scaled(float value);

// Draws a button and returns true when it is clicked
bool DrawButton(Rectangle rect, const char* text);

// Draws a button centered horizontally and returns true when it is clicked
bool DrawCenteredButton(float y, const char* text);

// Draws text centered horizontally on the screen
void DrawCenteredText(const char* text, float y, float fontSize, Color color);

// Fills the screen with dungeon floor tiles and darkens it
void DrawDungeonBackground();

// Draws a dark rectangle behind menus and dialogs
void DrawPanel(Rectangle rect);

// Adds the typed characters to "text" (up to maxLength) and handles backspace.
// With digitsOnly only 0-9 are accepted. Returns true if the text changed
bool UpdateTextInput(char* text, int maxLength, bool digitsOnly);

// Draws an input box with the text and a blinking cursor, red border on error
void DrawTextBox(Rectangle box, const char* text, float fontSize, bool hasError);

// Draws an animated torch with its top left corner at x, y
void DrawTorch(float x, float y, float size);
