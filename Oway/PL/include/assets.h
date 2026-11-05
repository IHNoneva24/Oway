#pragma once
#include "raylib.h"

// Size of one tile in the asset pack (in pixels)
const int TILE_SIZE = 16;

const int ANIMATION_FRAMES = 4;

struct GameAssets {
	Texture2D tileset;
	Texture2D characters;
	Texture2D torch[ANIMATION_FRAMES];
	Texture2D sideTorch[ANIMATION_FRAMES];
	Texture2D candlestick[ANIMATION_FRAMES];
	Texture2D flame[ANIMATION_FRAMES];  // lava cells in team mode
	Texture2D spikes[ANIMATION_FRAMES]; // frame 0 = up, frame 3 = down
};

// Loads all textures once at the start of the game
void LoadAssets();

// Frees all textures before the window closes
void UnloadAssets();

const GameAssets& GetAssets();

// Draws one 16x16 tile from a sprite sheet (column, row) stretched into the destination
void DrawTile(Texture2D sheet, int column, int row, Rectangle destination, bool flipX = false);

// Current frame (0-3) of the looping animations
int GetAnimationFrame();
