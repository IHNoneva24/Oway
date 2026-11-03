#include "screens.h"
#include "ui.h"
#include "assets.h"
#include "Game.h"
#include <ctime>
#include <cmath>

extern int screenW;
extern int screenH;

static const float HUD_HEIGHT = 70.0f;
static const float MAZE_MARGIN = 20.0f;

// Tiles from Dungeon_Tileset.png (column, row)
static const int FLOOR_TILES[3][2] = { { 6, 0 }, { 7, 0 }, { 8, 0 } };
static const int WALL_FACE_TILE[2] = { 2, 0 };
static const int WALL_TOP_TILE[2] = { 8, 7 };
static const int EXIT_TILE[2] = { 9, 3 };

// Reads the arrow keys and moves the player
static void HandleMovement(GameSession& session) {
	if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressedRepeat(KEY_W))       TryMovePlayer(session, DIRECTION_UP);
	if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressedRepeat(KEY_D)) TryMovePlayer(session, DIRECTION_RIGHT);
	if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressedRepeat(KEY_S))   TryMovePlayer(session, DIRECTION_DOWN);
	if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT) || IsKeyPressed(KEY_A) || IsKeyPressedRepeat(KEY_A))   TryMovePlayer(session, DIRECTION_LEFT);
}

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

// Size of one tile so the whole maze fits on the screen under the HUD
static float GetTileSize(const Maze& maze) {
	float availableWidth = screenW - Scaled(MAZE_MARGIN) * 2;
	float availableHeight = screenH - Scaled(HUD_HEIGHT) - Scaled(MAZE_MARGIN) * 2;
	float tileWidth = availableWidth / (maze.width * 2 + 1);
	float tileHeight = availableHeight / (maze.height * 2 + 1);

	// Whole pixels keep the pixel art sharp
	int size = static_cast<int>(tileWidth < tileHeight ? tileWidth : tileHeight);
	return static_cast<float>(size < 2 ? 2 : size);
}

// Draws the walls, the exit and the player, top-down view
static void DrawMaze(const GameSession& session) {
	const Maze& maze = session.maze;
	const GameAssets& assets = GetAssets();

	int tilesX = maze.width * 2 + 1;
	int tilesY = maze.height * 2 + 1;
	float tile = GetTileSize(maze);
	float offsetX = (screenW - tile * tilesX) / 2;
	float offsetY = Scaled(HUD_HEIGHT) + (screenH - Scaled(HUD_HEIGHT) - tile * tilesY) / 2;

	for (int tileY = 0; tileY < tilesY; tileY++) {
		for (int tileX = 0; tileX < tilesX; tileX++) {
			Rectangle destination = { offsetX + tileX * tile, offsetY + tileY * tile, tile, tile };

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

	Rectangle exitTile = { offsetX + (GetExitX(maze) * 2 + 1) * tile, offsetY + (GetExitY(maze) * 2 + 1) * tile, tile, tile };
	unsigned char glow = static_cast<unsigned char>(90 + 60 * sinf(static_cast<float>(GetTime()) * 4));
	DrawRectangleRec(exitTile, { 240, 180, 60, glow });
	DrawTile(assets.tileset, EXIT_TILE[0], EXIT_TILE[1], exitTile);

	// The knight sprite switches between two frames for an idle animation
	int frameRow = static_cast<int>(GetTime() * 2) % 2 == 0 ? 0 : 2;
	Rectangle playerTile = { offsetX + (session.player.x * 2 + 1) * tile, offsetY + (session.player.y * 2 + 1) * tile, tile, tile };
	DrawTile(assets.characters, 0, frameRow, playerTile, session.player.facing == DIRECTION_LEFT);
}

static void DrawHud(const GameSession& session) {
	float height = Scaled(HUD_HEIGHT);
	int fontSize = static_cast<int>(Scaled(30));
	int textY = static_cast<int>(height / 2 - fontSize / 2.0f);

	DrawRectangle(0, 0, screenW, static_cast<int>(height), COLOR_PANEL);

	DrawText(TextFormat("Maze: %d x %d", session.maze.width, session.maze.height), static_cast<int>(Scaled(30)), textY, fontSize, RAYWHITE);
	DrawText(TextFormat("Moves: %d", session.player.moves), static_cast<int>(Scaled(330)), textY, fontSize, RAYWHITE);
	DrawText(TextFormat("Time: %.1f s", session.elapsedSeconds), static_cast<int>(Scaled(580)), textY, fontSize, RAYWHITE);

	const char* hint = "ESC - pause";
	DrawText(hint, screenW - MeasureText(hint, fontSize) - static_cast<int>(Scaled(30)), textY, fontSize, COLOR_TEXT_DIM);
}

static void DrawPauseMenu(GameSession& session, Screen& currentScreen) {
	DrawRectangle(0, 0, screenW, screenH, { 0, 0, 0, 170 });

	float y = screenH * 0.3f;
	DrawCenteredText("PAUSED", y, Scaled(80), COLOR_ACCENT);

	y += Scaled(140);

	if (DrawCenteredButton(y, "Resume")) {
		SetPaused(session, false);
	}

	if (DrawCenteredButton(y + Scaled(95), "Restart (new maze)")) {
		RestartGameSession(session, static_cast<unsigned int>(time(nullptr)));
	}

	if (DrawCenteredButton(y + Scaled(190), "Main menu")) {
		EndGameSession(session);
		currentScreen = MAINMENU;
	}
}

void DrawGame(Screen& currentScreen) {
	GameSession& session = GetCurrentSession();

	if (IsKeyPressed(KEY_ESCAPE)) {
		SetPaused(session, !session.isPaused);
	}

	HandleMovement(session);
	UpdateGameTime(session, GetFrameTime());

	ClearBackground(COLOR_BACKGROUND);
	DrawMaze(session);
	DrawHud(session);

	if (session.isPaused) {
		DrawPauseMenu(session, currentScreen);
	}

	if (session.isFinished) {
		currentScreen = WIN;
	}
}
