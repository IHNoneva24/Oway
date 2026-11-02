#pragma once

// Maze logic

constexpr int MIN_MAZE_SIZE = 10;
constexpr int MAX_MAZE_SIZE = 50;

// Every cell stores its walls as bit flags
enum WallFlag : unsigned char {
	WALL_NORTH = 1,
	WALL_EAST = 2,
	WALL_SOUTH = 4,
	WALL_WEST = 8,
	WALL_ALL = WALL_NORTH | WALL_EAST | WALL_SOUTH | WALL_WEST
};

enum Direction {
	DIRECTION_UP,
	DIRECTION_RIGHT,
	DIRECTION_DOWN,
	DIRECTION_LEFT
};

struct Maze {
	int width = 0;
	int height = 0;
	unsigned char* cells = nullptr; // dynamic array of width * height wall flags
};

// Checks if a size is inside the allowed range (10 - 50)
bool IsValidMazeSize(int size);

// Allocates the cells with all walls closed. Returns false on invalid size
bool CreateMaze(Maze& maze, int width, int height);

// Frees the dynamic memory of the maze
void DestroyMaze(Maze& maze);

// Carves a perfect maze (one path between any two cells). Same seed = same maze
void GenerateMaze(Maze& maze, unsigned int seed);

bool IsInsideMaze(const Maze& maze, int x, int y);

bool HasWall(const Maze& maze, int x, int y, Direction direction);

// The exit is always the bottom right cell
int GetExitX(const Maze& maze);
int GetExitY(const Maze& maze);
