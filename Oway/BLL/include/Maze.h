#pragma once

// constant
constexpr int MIN_MAZE_SIZE = 10;
constexpr int MAX_MAZE_SIZE = 50;

// Wall bits
enum WallFlag : unsigned char {
	WALL_NORTH = 1,
	WALL_EAST = 2,
	WALL_SOUTH = 4,
	WALL_WEST = 8,
	WALL_ALL = WALL_NORTH | WALL_EAST | WALL_SOUTH | WALL_WEST
};

// Directions
enum Direction {
	DIRECTION_UP,
	DIRECTION_RIGHT,
	DIRECTION_DOWN,
	DIRECTION_LEFT
};

// Maze data
struct Maze {
	int width = 0;
	int height = 0;
	unsigned char* cells = nullptr;
};

// Size check
bool IsValidMazeSize(int size);

// Read size
bool TryParseMazeSize(const char* text, int& size);

// Create maze
bool CreateMaze(Maze& maze, int width, int height);

// Free maze
void DestroyMaze(Maze& maze);

// Generate maze
void GenerateMaze(Maze& maze, unsigned int seed);

// Inside check
bool IsInsideMaze(const Maze& maze, int x, int y);

// Wall check
bool HasWall(const Maze& maze, int x, int y, Direction direction);

// Exit column
int GetExitX(const Maze& maze);
// Exit row
int GetExitY(const Maze& maze);
