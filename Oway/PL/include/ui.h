#pragma once
#include "raylib.h"

const Color COLOR_BACKGROUND = { 26, 18, 30, 255 };
const Color COLOR_PANEL = { 20, 14, 24, 220 };
const Color COLOR_BUTTON = { 92, 60, 52, 235 };
const Color COLOR_BUTTON_HOVER = { 140, 90, 66, 250 };
const Color COLOR_ACCENT = { 240, 180, 60, 255 };
const Color COLOR_ERROR = { 230, 80, 70, 255 };
const Color COLOR_TEXT_DIM = { 170, 160, 170, 255 };

// UI scale
float GetUiScale();

// Scaled value
float Scaled(float value);

// Draw button
bool DrawButton(Rectangle rect, const char* text);

// Centered button
bool DrawCenteredButton(float y, const char* text);

// Centered text
void DrawCenteredText(const char* text, float y, float fontSize, Color color);

// Draw background
void DrawDungeonBackground();

// Draw panel
void DrawPanel(Rectangle rect);

// Text input
bool UpdateTextInput(char* text, int maxLength, bool digitsOnly);

// Filtered input
bool UpdateFilteredTextInput(char* text, int maxLength, const char* allowedCharacters);

// Draw input box
void DrawTextBox(Rectangle box, const char* text, float fontSize, bool hasError);

// Draw torch
void DrawTorch(float x, float y, float size);
