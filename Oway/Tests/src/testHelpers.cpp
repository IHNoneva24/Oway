#include "testHelpers.h"

static const int DIRECTION_DX[4] = { 0, 1, 0, -1 };
static const int DIRECTION_DY[4] = { -1, 0, 1, 0 };

int CountReachableCells(const Maze& maze) {
	int cellCount = maze.width * maze.height;
	bool* visited = new bool[cellCount];
	int* queue = new int[cellCount];
	int head = 0;
	int tail = 0;

	for (int i = 0; i < cellCount; i++) {
		visited[i] = false;
	}

	visited[0] = true;
	queue[tail++] = 0;

	// Breadth-first search from the start
	while (head < tail) {
		int current = queue[head++];
		int x = current % maze.width;
		int y = current / maze.width;

		for (int dir = 0; dir < 4; dir++) {
			if (HasWall(maze, x, y, static_cast<Direction>(dir))) {
				continue;
			}

			int next = (y + DIRECTION_DY[dir]) * maze.width + (x + DIRECTION_DX[dir]);

			if (!visited[next]) {
				visited[next] = true;
				queue[tail++] = next;
			}
		}
	}

	delete[] visited;
	delete[] queue;
	return tail;
}

int CountPassages(const Maze& maze) {
	int passages = 0;

	// Only right and down, so every opening is counted once
	for (int y = 0; y < maze.height; y++) {
		for (int x = 0; x < maze.width; x++) {
			if (x + 1 < maze.width && !HasWall(maze, x, y, DIRECTION_RIGHT)) passages++;
			if (y + 1 < maze.height && !HasWall(maze, x, y, DIRECTION_DOWN)) passages++;
		}
	}

	return passages;
}

Direction GetOppositeDirection(Direction direction) {
	return static_cast<Direction>((direction + 2) % 4);
}

bool FindOpenNeighbour(const Maze& maze, int x, int y, int& neighbourX, int& neighbourY, Direction& towardsCell) {
	for (int dir = 0; dir < 4; dir++) {
		if (!HasWall(maze, x, y, static_cast<Direction>(dir))) {
			neighbourX = x + DIRECTION_DX[dir];
			neighbourY = y + DIRECTION_DY[dir];
			towardsCell = GetOppositeDirection(static_cast<Direction>(dir));
			return true;
		}
	}

	return false;
}
