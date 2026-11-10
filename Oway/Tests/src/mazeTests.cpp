#include "tests.h"
#include "testFramework.h"
#include "testHelpers.h"
#include "Maze.h"

// Size limits: only 10 - 50 is allowed
static void TestSizeValidation() {
	Check(!IsValidMazeSize(9), "size 9 is rejected (below the minimum)");
	Check(IsValidMazeSize(10), "size 10 is accepted (minimum)");
	Check(IsValidMazeSize(50), "size 50 is accepted (maximum)");
	Check(!IsValidMazeSize(51), "size 51 is rejected (above the maximum)");
	Check(!IsValidMazeSize(-10), "negative size is rejected");
}

// Text typed by the user in the size box
static void TestSizeParsing() {
	int size = 0;

	Check(TryParseMazeSize("25", size) && size == 25, "\"25\" is read as 25");
	Check(TryParseMazeSize("10", size) && size == 10, "\"10\" is read as 10");
	Check(!TryParseMazeSize("", size), "empty text is rejected");
	Check(!TryParseMazeSize(nullptr, size), "missing text (nullptr) is rejected");
	Check(!TryParseMazeSize("5", size), "\"5\" is rejected (too small)");
	Check(!TryParseMazeSize("99", size), "\"99\" is rejected (too big)");
	Check(!TryParseMazeSize("2a", size), "\"2a\" is rejected (not a number)");
	Check(!TryParseMazeSize("-20", size), "\"-20\" is rejected (minus sign)");
	Check(!TryParseMazeSize("1000", size), "\"1000\" is rejected (too many digits)");

	size = 33;
	TryParseMazeSize("abc", size);
	CheckEqual(33, size, "the size is not changed when the text is wrong");
}

static void TestCreateAndDestroy() {
	Maze maze;

	Check(!CreateMaze(maze, 5, 5), "a 5x5 maze can't be created");
	Check(maze.cells == nullptr, "no memory is taken for a wrong size");

	Check(CreateMaze(maze, 12, 20), "a 12x20 maze is created");
	Check(maze.cells != nullptr, "the cells array is allocated");
	CheckEqual(12, maze.width, "the width is stored");
	CheckEqual(20, maze.height, "the height is stored");

	bool allClosed = true;

	for (int i = 0; i < maze.width * maze.height; i++) {
		allClosed = allClosed && maze.cells[i] == WALL_ALL;
	}

	Check(allClosed, "a new maze has every wall closed");

	DestroyMaze(maze);
	Check(maze.cells == nullptr && maze.width == 0 && maze.height == 0, "DestroyMaze frees the memory and resets the size");

	DestroyMaze(maze);
	Check(maze.cells == nullptr, "DestroyMaze can be called twice safely");
}

// A perfect maze: every cell can be reached and there is exactly one way between any two cells
static void TestPerfectMaze() {
	bool allReachable = true;
	bool noLoops = true;

	for (int size = MIN_MAZE_SIZE; size <= MAX_MAZE_SIZE; size++) {
		Maze maze;
		CreateMaze(maze, size, size);
		GenerateMaze(maze, 1000u + size);

		int cellCount = size * size;
		allReachable = allReachable && CountReachableCells(maze) == cellCount;

		// A connected maze without loops has exactly (cells - 1) openings
		noLoops = noLoops && CountPassages(maze) == cellCount - 1;

		DestroyMaze(maze);
	}

	Check(allReachable, "every cell is reachable from the start, for every size 10 - 50");
	Check(noLoops, "the maze has no loops (cells - 1 openings), for every size 10 - 50");
}

static void TestWalls() {
	Maze maze;
	CreateMaze(maze, 15, 15);
	GenerateMaze(maze, 42u);

	bool outerWallsClosed = true;
	bool wallsMatch = true;

	for (int i = 0; i < 15; i++) {
		outerWallsClosed = outerWallsClosed
			&& HasWall(maze, i, 0, DIRECTION_UP) && HasWall(maze, i, 14, DIRECTION_DOWN)
			&& HasWall(maze, 0, i, DIRECTION_LEFT) && HasWall(maze, 14, i, DIRECTION_RIGHT);
	}

	// Every wall must look the same from both sides
	for (int y = 0; y < 15; y++) {
		for (int x = 0; x < 14; x++) {
			wallsMatch = wallsMatch && HasWall(maze, x, y, DIRECTION_RIGHT) == HasWall(maze, x + 1, y, DIRECTION_LEFT);
			wallsMatch = wallsMatch && HasWall(maze, y, x, DIRECTION_DOWN) == HasWall(maze, y, x + 1, DIRECTION_UP);
		}
	}

	Check(outerWallsClosed, "the outer walls are always closed (the player can't leave the maze)");
	Check(wallsMatch, "a wall seen from one cell is also there from the neighbouring cell");
	Check(HasWall(maze, -1, 0, DIRECTION_RIGHT), "a position outside the maze counts as a wall");
	Check(!IsInsideMaze(maze, 15, 0) && IsInsideMaze(maze, 14, 14), "IsInsideMaze knows the borders");
	Check(GetExitX(maze) == 14 && GetExitY(maze) == 14, "the exit is in the bottom right corner");

	DestroyMaze(maze);
}

// The same seed must give the same maze (needed for LAN games), different seeds different mazes
static void TestSeeds() {
	Maze first;
	Maze second;
	Maze third;
	CreateMaze(first, 20, 20);
	CreateMaze(second, 20, 20);
	CreateMaze(third, 20, 20);
	GenerateMaze(first, 7u);
	GenerateMaze(second, 7u);
	GenerateMaze(third, 8u);

	bool isSame = true;
	bool isDifferent = false;

	for (int i = 0; i < 400; i++) {
		isSame = isSame && first.cells[i] == second.cells[i];
		isDifferent = isDifferent || first.cells[i] != third.cells[i];
	}

	Check(isSame, "the same seed gives exactly the same maze");
	Check(isDifferent, "a different seed gives a different maze");

	DestroyMaze(first);
	DestroyMaze(second);
	DestroyMaze(third);
}

void RunMazeTests() {
	BeginTestGroup("Maze (BLL/Maze)");

	TestSizeValidation();
	TestSizeParsing();
	TestCreateAndDestroy();
	TestPerfectMaze();
	TestWalls();
	TestSeeds();
}
