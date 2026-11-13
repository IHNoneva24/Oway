#pragma once
#include "Maze.h"

// Reachable cells
int CountReachableCells(const Maze& maze);

// Count openings
int CountPassages(const Maze& maze);

// Open neighbour
bool FindOpenNeighbour(const Maze& maze, int x, int y, int& neighbourX, int& neighbourY, Direction& towardsCell);

// Opposite direction
Direction GetOppositeDirection(Direction direction);
