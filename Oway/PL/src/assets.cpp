#include "assets.h"

static GameAssets assets;

static const float ANIMATION_SPEED = 8.0f; // frames per second

void LoadAssets() {
	assets.tileset = LoadTexture("assets/tileset/Dungeon_Tileset.png");
	assets.characters = LoadTexture("assets/tileset/Dungeon_Character.png");

	for (int i = 0; i < ANIMATION_FRAMES; i++) {
		assets.torch[i] = LoadTexture(TextFormat("assets/items/torch/torch_%d.png", i + 1));
		assets.sideTorch[i] = LoadTexture(TextFormat("assets/items/torch/side_torch_%d.png", i + 1));
		assets.candlestick[i] = LoadTexture(TextFormat("assets/items/torch/candlestick_1_%d.png", i + 1));
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
