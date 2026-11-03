#include "Maze.h"
#include <random>

// Offsets and wall flags for each direction (same order as the Direction enum)
static const int DIRECTION_DX[4] = { 0, 1, 0, -1 };
static const int DIRECTION_DY[4] = { -1, 0, 1, 0 };
static const unsigned char DIRECTION_WALL[4] = { WALL_NORTH, WALL_EAST, WALL_SOUTH, WALL_WEST };
static const unsigned char OPPOSITE_WALL[4] = { WALL_SOUTH, WALL_WEST, WALL_NORTH, WALL_EAST };

static int GetCellIndex(const Maze& maze, int x, int y) {
	return y * maze.width + x;
}

bool IsValidMazeSize(int size) {
	return size >= MIN_MAZE_SIZE && size <= MAX_MAZE_SIZE;
}

bool TryParseMazeSize(const char* text, int& size) {
	if (text == nullptr || text[0] == '\0') {
		return false;
	}

	int value = 0;

	for (int i = 0; text[i] != '\0'; i++) {
		if (text[i] < '0' || text[i] > '9' || i >= 3) {
			return false;
		}

		value = value * 10 + (text[i] - '0');
	}

	if (!IsValidMazeSize(value)) {
		return false;
	}

	size = value;
	return true;
}

bool CreateMaze(Maze& maze, int width, int height) {
	if (!IsValidMazeSize(width) || !IsValidMazeSize(height)) {
		return false;
	}

	DestroyMaze(maze);

	maze.width = width;
	maze.height = height;
	maze.cells = new unsigned char[width * height];

	for (int i = 0; i < width * height; i++) {
		maze.cells[i] = WALL_ALL;
	}

	return true;
}

void DestroyMaze(Maze& maze) {
	delete[] maze.cells;
	maze.cells = nullptr;
	maze.width = 0;
	maze.height = 0;
}

void GenerateMaze(Maze& maze, unsigned int seed) {
	if (maze.cells == nullptr) {
		return;
	}

	int cellCount = maze.width * maze.height;
	std::mt19937 random(seed);

	// Iterative depth-first search (recursive backtracker) with our own stack
	bool* visited = new bool[cellCount];
	int* stack = new int[cellCount];
	int stackSize = 0;

	for (int i = 0; i < cellCount; i++) {
		maze.cells[i] = WALL_ALL;
		visited[i] = false;
	}

	visited[0] = true;
	stack[stackSize++] = 0;

	while (stackSize > 0) {
		int current = stack[stackSize - 1];
		int x = current % maze.width;
		int y = current / maze.width;

		// Collect the directions that lead to unvisited neighbours
		int options[4];
		int optionCount = 0;

		for (int dir = 0; dir < 4; dir++) {
			int nx = x + DIRECTION_DX[dir];
			int ny = y + DIRECTION_DY[dir];

			if (IsInsideMaze(maze, nx, ny) && !visited[GetCellIndex(maze, nx, ny)]) {
				options[optionCount++] = dir;
			}
		}

		if (optionCount == 0) {
			stackSize--; // dead end - go back
			continue;
		}

		int dir = options[random() % optionCount];
		int next = GetCellIndex(maze, x + DIRECTION_DX[dir], y + DIRECTION_DY[dir]);

		// Break the wall between the current and the next cell from both sides
		maze.cells[current] &= ~DIRECTION_WALL[dir];
		maze.cells[next] &= ~OPPOSITE_WALL[dir];

		visited[next] = true;
		stack[stackSize++] = next;
	}

	delete[] visited;
	delete[] stack;
}

bool IsInsideMaze(const Maze& maze, int x, int y) {
	return x >= 0 && y >= 0 && x < maze.width && y < maze.height;
}

bool HasWall(const Maze& maze, int x, int y, Direction direction) {
	if (!IsInsideMaze(maze, x, y)) {
		return true;
	}

	return (maze.cells[GetCellIndex(maze, x, y)] & DIRECTION_WALL[direction]) != 0;
}

int GetExitX(const Maze& maze) {
	return maze.width - 1;
}

int GetExitY(const Maze& maze) {
	return maze.height - 1;
}
