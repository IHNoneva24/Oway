#pragma once
#include "raylib.h"

const int TILE_SIZE = 16;

const int ANIMATION_FRAMES = 4;

// Game textures
struct GameAssets {
	Texture2D tileset;
	Texture2D characters;
	Texture2D torch[ANIMATION_FRAMES];
	Texture2D sideTorch[ANIMATION_FRAMES];
	Texture2D candlestick[ANIMATION_FRAMES];
	Texture2D flame[ANIMATION_FRAMES];
	Texture2D spikes[ANIMATION_FRAMES];
};

// Load textures
void LoadAssets();

// Free textures
void UnloadAssets();

// All textures
const GameAssets& GetAssets();

// Draw tile
void DrawTile(Texture2D sheet, int column, int row, Rectangle destination, bool flipX = false);

// Animation frame
int GetAnimationFrame();
