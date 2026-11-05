#include "mazeView.h"
#include "ui.h"
#include "assets.h"
#include <cmath>

extern int screenW;
extern int screenH;

static const float MAZE_MARGIN = 20.0f;

// Tiles from Dungeon_Tileset.png (column, row)
static const int FLOOR_TILES[3][2] = { { 6, 0 }, { 7, 0 }, { 8, 0 } };
static const int WALL_FACE_TILE[2] = { 2, 0 };
static const int WALL_TOP_TILE[2] = { 8, 7 };
static const int EXIT_TILE[2] = { 9, 3 };

// The maze is drawn as a grid of (2 * width + 1) x (2 * height + 1) tiles:
// odd tiles are the cells, the tiles between them are walls or openings
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

float GetTileSize(const Maze& maze) {
	float availableWidth = screenW - Scaled(MAZE_MARGIN) * 2;
	float availableHeight = screenH - Scaled(HUD_HEIGHT) - Scaled(MAZE_MARGIN) * 2;
	float tileWidth = availableWidth / (maze.width * 2 + 1);
	float tileHeight = availableHeight / (maze.height * 2 + 1);

	// Whole pixels keep the pixel art sharp
	int size = static_cast<int>(tileWidth < tileHeight ? tileWidth : tileHeight);
	return static_cast<float>(size < 2 ? 2 : size);
}

// Top left corner of the maze, centered under the HUD
static Vector2 GetMazeOffset(const Maze& maze, float tile) {
	float offsetX = (screenW - tile * (maze.width * 2 + 1)) / 2;
	float offsetY = Scaled(HUD_HEIGHT) + (screenH - Scaled(HUD_HEIGHT) - tile * (maze.height * 2 + 1)) / 2;
	return { offsetX, offsetY };
}

Rectangle GetCellRect(const Maze& maze, int x, int y) {
	float tile = GetTileSize(maze);
	Vector2 offset = GetMazeOffset(maze, tile);

	return { offset.x + (x * 2 + 1) * tile, offset.y + (y * 2 + 1) * tile, tile, tile };
}

void DrawMazeWalls(const Maze& maze) {
	const GameAssets& assets = GetAssets();

	int tilesX = maze.width * 2 + 1;
	int tilesY = maze.height * 2 + 1;
	float tile = GetTileSize(maze);
	Vector2 offset = GetMazeOffset(maze, tile);

	for (int tileY = 0; tileY < tilesY; tileY++) {
		for (int tileX = 0; tileX < tilesX; tileX++) {
			Rectangle destination = { offset.x + tileX * tile, offset.y + tileY * tile, tile, tile };

			if (IsFloorTile(maze, tileX, tileY)) {
				const int* floor = FLOOR_TILES[(tileX * 7 + tileY * 3) % 3];
				DrawTile(assets.tileset, floor[0], floor[1], destination);
				continue;
			}

			// A wall with floor under it shows its brick front side
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

void DrawExit(const Maze& maze, bool isOpen) {
	Rectangle exitTile = GetCellRect(maze, GetExitX(maze), GetExitY(maze));

	if (isOpen) {
		unsigned char glow = static_cast<unsigned char>(90 + 60 * sinf(static_cast<float>(GetTime()) * 4));
		DrawRectangleRec(exitTile, { 240, 180, 60, glow });
	}

	DrawTile(GetAssets().tileset, EXIT_TILE[0], EXIT_TILE[1], exitTile);

	if (!isOpen) {
		DrawRectangleRec(exitTile, { 0, 0, 0, 150 });
	}
}
