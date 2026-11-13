#pragma once
#include "raylib.h"
#include "Maze.h"

const float HUD_HEIGHT = 70.0f;

// Tile size
float GetTileSize(const Maze& maze);

// Cell rectangle
Rectangle GetCellRect(const Maze& maze, int x, int y);

// Draw walls
void DrawMazeWalls(const Maze& maze);

// Draw exit
void DrawExit(const Maze& maze, bool isOpen);

// Tile size
float GetTileSizeIn(const Maze& maze, Rectangle area);
// Cell rectangle
Rectangle GetCellRectIn(const Maze& maze, Rectangle area, int x, int y);
// Draw walls
void DrawMazeWallsIn(const Maze& maze, Rectangle area);
// Draw exit
void DrawExitIn(const Maze& maze, Rectangle area, bool isOpen);
