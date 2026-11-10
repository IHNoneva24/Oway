#pragma once
#include "Maze.h"

// Helpers used by several test groups

// How many cells can be reached from 0,0 without going through walls
int CountReachableCells(const Maze& maze);

// How many openings (removed walls) the maze has between neighbouring cells
int CountPassages(const Maze& maze);

// Finds a neighbour of (x, y) with no wall between them.
// "towardsCell" is the direction to walk from the neighbour back to (x, y)
bool FindOpenNeighbour(const Maze& maze, int x, int y, int& neighbourX, int& neighbourY, Direction& towardsCell);

// Opposite direction (up <-> down, left <-> right)
Direction GetOppositeDirection(Direction direction);
