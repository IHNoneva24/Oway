#include "mazeView.h"
#include "ui.h"
#include "assets.h"
#include <cmath>

// from screen.cpp
extern int screenW;
extern int screenH;

static const float MAZE_MARGIN = 20.0f;

static const int FLOOR_TILES[3][2] = { { 6, 0 }, { 7, 0 }, { 8, 0 } };
static const int WALL_FACE_TILE[2] = { 2, 0 };
static const int WALL_TOP_TILE[2] = { 8, 7 };
static const int EXIT_TILE[2] = { 9, 3 };

// Floor check
static bool IsFloorTile(const Maze& maze, int tileX, int tileY) {
	bool isOddX = tileX % 2 == 1;
	bool isOddY = tileY % 2 == 1;

	if (isOddX && isOddY) {
		return true;
	}

	if (!isOddX && isOddY) {
		return !HasWall(maze, tileX / 2 - 1, tileY / 2, DIRECTION_RIGHT) || !HasWall(maze, tileX / 2, tileY / 2, DIRECTION_LEFT);
	}

	if (isOddX && !isOddY) {
		return !HasWall(maze, tileX / 2, tileY / 2 - 1, DIRECTION_DOWN) || !HasWall(maze, tileX / 2, tileY / 2, DIRECTION_UP);
	}

	return false;
}

// Screen area
static Rectangle GetFullScreenArea() {
	float margin = Scaled(MAZE_MARGIN);
	float top = Scaled(HUD_HEIGHT) + margin;
	return { margin, top, screenW - margin * 2, screenH - top - margin };
}

// Tile size
float GetTileSizeIn(const Maze& maze, Rectangle area) {
	float tileWidth = area.width / (maze.width * 2 + 1);
	float tileHeight = area.height / (maze.height * 2 + 1);

	int size = static_cast<int>(tileWidth < tileHeight ? tileWidth : tileHeight);
	return static_cast<float>(size < 2 ? 2 : size);
}

// Tile size
float GetTileSize(const Maze& maze) {
	return GetTileSizeIn(maze, GetFullScreenArea());
}

// Maze position
static Vector2 GetMazeOffset(const Maze& maze, Rectangle area, float tile) {
	float offsetX = area.x + (area.width - tile * (maze.width * 2 + 1)) / 2;
	float offsetY = area.y + (area.height - tile * (maze.height * 2 + 1)) / 2;
	return { offsetX, offsetY };
}

// Cell rectangle
Rectangle GetCellRectIn(const Maze& maze, Rectangle area, int x, int y) {
	float tile = GetTileSizeIn(maze, area);
	Vector2 offset = GetMazeOffset(maze, area, tile);

	return { offset.x + (x * 2 + 1) * tile, offset.y + (y * 2 + 1) * tile, tile, tile };
}

// Cell rectangle
Rectangle GetCellRect(const Maze& maze, int x, int y) {
	return GetCellRectIn(maze, GetFullScreenArea(), x, y);
}

// Draw walls
void DrawMazeWalls(const Maze& maze) {
	DrawMazeWallsIn(maze, GetFullScreenArea());
}

// Draw walls
void DrawMazeWallsIn(const Maze& maze, Rectangle area) {
	const GameAssets& assets = GetAssets();

	int tilesX = maze.width * 2 + 1;
	int tilesY = maze.height * 2 + 1;
	float tile = GetTileSizeIn(maze, area);
	Vector2 offset = GetMazeOffset(maze, area, tile);

	for (int tileY = 0; tileY < tilesY; tileY++) {
		for (int tileX = 0; tileX < tilesX; tileX++) {
			Rectangle destination = { offset.x + tileX * tile, offset.y + tileY * tile, tile, tile };

			if (IsFloorTile(maze, tileX, tileY)) {
				const int* floor = FLOOR_TILES[(tileX * 7 + tileY * 3) % 3];
				DrawTile(assets.tileset, floor[0], floor[1], destination);
				continue;
			}

			bool hasFloorBelow = tileY + 1 < tilesY && IsFloorTile(maze, tileX, tileY + 1);

			if (hasFloorBelow) {
				DrawTile(assets.tileset, WALL_FACE_TILE[0], WALL_FACE_TILE[1], destination);

				if ((tileX * 7 + tileY * 13) % 11 == 0) {
					Texture2D torch = assets.sideTorch[GetAnimationFrame()];
					DrawTexturePro(torch, { 0, 0, static_cast<float>(torch.width), static_cast<float>(torch.height) },
						destination, { 0, 0 }, 0.0f, WHITE);
				}
			}
			else {
				DrawTile(assets.tileset, WALL_TOP_TILE[0], WALL_TOP_TILE[1], destination);
			}
		}
	}
}

// Draw exit
void DrawExit(const Maze& maze, bool isOpen) {
	DrawExitIn(maze, GetFullScreenArea(), isOpen);
}

// Draw exit
void DrawExitIn(const Maze& maze, Rectangle area, bool isOpen) {
	Rectangle exitTile = GetCellRectIn(maze, area, GetExitX(maze), GetExitY(maze));

	if (isOpen) {
		// pulsing glow
		unsigned char glow = static_cast<unsigned char>(90 + 60 * sinf(static_cast<float>(GetTime()) * 4));
		DrawRectangleRec(exitTile, { 240, 180, 60, glow });
	}

	DrawTile(GetAssets().tileset, EXIT_TILE[0], EXIT_TILE[1], exitTile);

	if (!isOpen) {
		DrawRectangleRec(exitTile, { 0, 0, 0, 150 });
	}
}
