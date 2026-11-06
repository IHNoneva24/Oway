#pragma once
#include "raylib.h"
#include "Maze.h"

// Drawing of the maze, shared by the single player and the team game screens

const float HUD_HEIGHT = 70.0f;

// Size of one tile so the whole maze fits on the screen under the HUD
float GetTileSize(const Maze& maze);

// Position and size of a maze cell on the screen
Rectangle GetCellRect(const Maze& maze, int x, int y);

// Draws the floor and the walls, top-down view
void DrawMazeWalls(const Maze& maze);

// Draws the ladder in the bottom right corner. A closed exit is dark and doesn't glow
void DrawExit(const Maze& maze, bool isOpen);

// The same functions for a maze drawn inside a part of the screen (split screen)
float GetTileSizeIn(const Maze& maze, Rectangle area);
Rectangle GetCellRectIn(const Maze& maze, Rectangle area, int x, int y);
void DrawMazeWallsIn(const Maze& maze, Rectangle area);
void DrawExitIn(const Maze& maze, Rectangle area, bool isOpen);
