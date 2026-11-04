#include "assets.h"

static GameAssets assets;

static const float ANIMATION_SPEED = 8.0f; // frames per second

// Assets are loaded from the folder of the executable, so the game works from any working directory
static Texture2D LoadGameTexture(const char* relativePath) {
	return LoadTexture(TextFormat("%sassets/%s", GetApplicationDirectory(), relativePath));
}

void LoadAssets() {
	assets.tileset = LoadGameTexture("tileset/Dungeon_Tileset.png");
	assets.characters = LoadGameTexture("tileset/Dungeon_Character.png");

	for (int i = 0; i < ANIMATION_FRAMES; i++) {
		assets.torch[i] = LoadGameTexture(TextFormat("items/torch/torch_%d.png", i + 1));
		assets.sideTorch[i] = LoadGameTexture(TextFormat("items/torch/side_torch_%d.png", i + 1));
		assets.candlestick[i] = LoadGameTexture(TextFormat("items/torch/candlestick_1_%d.png", i + 1));
	}
}

void UnloadAssets() {
	UnloadTexture(assets.tileset);
	UnloadTexture(assets.characters);

	for (int i = 0; i < ANIMATION_FRAMES; i++) {
		UnloadTexture(assets.torch[i]);
		UnloadTexture(assets.sideTorch[i]);
		UnloadTexture(assets.candlestick[i]);
	}
}

const GameAssets& GetAssets() {
	return assets;
}

void DrawTile(Texture2D sheet, int column, int row, Rectangle destination, bool flipX) {
	float width = static_cast<float>(TILE_SIZE);
	Rectangle source = { column * width, row * width, flipX ? -width : width, width };

	DrawTexturePro(sheet, source, destination, { 0, 0 }, 0.0f, WHITE);
}

int GetAnimationFrame() {
	return static_cast<int>(GetTime() * ANIMATION_SPEED) % ANIMATION_FRAMES;
}
