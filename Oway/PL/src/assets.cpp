#include "assets.h"

static GameAssets assets;

static const float ANIMATION_SPEED = 8.0f; // frames per second

// Assets are loaded from the folder of the executable, so the game works from any working directory
static Texture2D LoadGameTexture(const char* relativePath) {
	return LoadTexture(TextFormat("%sassets/%s", GetApplicationDirectory(), relativePath));
}

// The character sheet marks every sprite with small blue corners. They are removed when the sheet is loaded,
// only in the corners, because the same blue is used inside some sprites
static Texture2D LoadCharacterSheet(const char* relativePath) {
	const Color MARKER = { 98, 171, 212, 255 };
	const int CORNER = 3;

	Image image = LoadImage(TextFormat("%sassets/%s", GetApplicationDirectory(), relativePath));
	ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	Color* pixels = static_cast<Color*>(image.data);

	for (int y = 0; y < image.height; y++) {
		for (int x = 0; x < image.width; x++) {
			int cellX = x % TILE_SIZE;
			int cellY = y % TILE_SIZE;
			bool isCorner = (cellX < CORNER || cellX >= TILE_SIZE - CORNER) && (cellY < CORNER || cellY >= TILE_SIZE - CORNER);
			Color& pixel = pixels[y * image.width + x];

			if (isCorner && pixel.r == MARKER.r && pixel.g == MARKER.g && pixel.b == MARKER.b) {
				pixel = BLANK;
			}
		}
	}

	Texture2D texture = LoadTextureFromImage(image);
	UnloadImage(image);
	return texture;
}

void LoadAssets() {
	assets.tileset = LoadGameTexture("tileset/Dungeon_Tileset.png");
	assets.characters = LoadCharacterSheet("tileset/Dungeon_Character.png");

	for (int i = 0; i < ANIMATION_FRAMES; i++) {
		assets.torch[i] = LoadGameTexture(TextFormat("items/torch/torch_%d.png", i + 1));
		assets.sideTorch[i] = LoadGameTexture(TextFormat("items/torch/side_torch_%d.png", i + 1));
		assets.candlestick[i] = LoadGameTexture(TextFormat("items/torch/candlestick_1_%d.png", i + 1));
		assets.flame[i] = LoadGameTexture(TextFormat("items/flamethrower/flamethrower_1_%d.png", i + 1));
		assets.spikes[i] = LoadGameTexture(TextFormat("items/peaks/peaks_%d.png", i + 1));
	}
}

void UnloadAssets() {
	UnloadTexture(assets.tileset);
	UnloadTexture(assets.characters);

	for (int i = 0; i < ANIMATION_FRAMES; i++) {
		UnloadTexture(assets.torch[i]);
		UnloadTexture(assets.sideTorch[i]);
		UnloadTexture(assets.candlestick[i]);
		UnloadTexture(assets.flame[i]);
		UnloadTexture(assets.spikes[i]);
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
