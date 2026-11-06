#include "TeamGame.h"
#include <random>

static TeamSession currentTeamSession;

// Offsets for each direction (same order as the Direction enum)
static const int DIRECTION_DX[4] = { 0, 1, 0, -1 };
static const int DIRECTION_DY[4] = { -1, 0, 1, 0 };

static int GetCellIndex(const Maze& maze, int x, int y) {
	return y * maze.width + x;
}

// Breadth-first search from the start (0,0).
// parent[i] is the cell we came from to reach cell i (-1 for the start)
static void FindParents(const Maze& maze, int* parent) {
	int cellCount = maze.width * maze.height;
	int* queue = new int[cellCount];
	int head = 0;
	int tail = 0;

	for (int i = 0; i < cellCount; i++) {
		parent[i] = -2; // not visited yet
	}

	parent[0] = -1;
	queue[tail++] = 0;

	while (head < tail) {
		int current = queue[head++];
		int x = current % maze.width;
		int y = current / maze.width;

		for (int dir = 0; dir < 4; dir++) {
			if (HasWall(maze, x, y, static_cast<Direction>(dir))) {
				continue;
			}

			int next = GetCellIndex(maze, x + DIRECTION_DX[dir], y + DIRECTION_DY[dir]);

			if (parent[next] == -2) {
				parent[next] = current;
				queue[tail++] = next;
			}
		}
	}

	delete[] queue;
}

// Gates are put on the only way to the exit. Each gate has a plate 2 cells before and after it,
// so one hero holds a plate while the other one walks through
static void PlaceGates(TeamSession& session, const int* path, int pathLength) {
	int gateCount = session.maze.width / 10;
	int spacing = pathLength / (gateCount + 1);

	// The gates need enough space between them for their plates
	while (gateCount > 0 && spacing < 6) {
		gateCount--;
		spacing = pathLength / (gateCount + 1);
	}

	for (int i = 1; i <= gateCount; i++) {
		int position = spacing * i;
		session.cells[path[position]] = CELL_GATE;
		session.cells[path[position - 2]] = CELL_PLATE;
		session.cells[path[position + 2]] = CELL_PLATE;
	}
}

// Spikes are put on free cells of the way to the exit (never on the start or the exit).
// Two spikes are never next to each other: neighbouring spikes go up and down at opposite times,
// so a hero standing on one of them could never step onto the other one
static void PlaceSpikes(TeamSession& session, const int* path, int pathLength, std::mt19937& random) {
	int spikeCount = pathLength / 8;

	for (int attempt = 0; attempt < spikeCount * 10 && spikeCount > 0; attempt++) {
		int position = 2 + random() % (pathLength - 4);
		bool hasSpikeNext = session.cells[path[position - 1]] == CELL_SPIKES
			|| session.cells[path[position + 1]] == CELL_SPIKES;

		if (session.cells[path[position]] == CELL_EMPTY && !hasSpikeNext) {
			session.cells[path[position]] = CELL_SPIKES;
			spikeCount--;
		}
	}
}

// Gems are put at dead ends of side ways. The first cell of that side way becomes lava (fire gem)
// or water (water gem), so only the right hero can get the gem
static void PlaceGems(TeamSession& session, const int* parent, const bool* isOnPath, std::mt19937& random) {
	const Maze& maze = session.maze;
	int cellCount = maze.width * maze.height;
	int gemsPerHero = maze.width / 10;
	int fireGems = 0;
	int waterGems = 0;

	// A dead end is a cell with only one opening
	int* deadEnds = new int[cellCount];
	int deadEndCount = 0;

	for (int i = 1; i < cellCount; i++) {
		int openings = 0;

		for (int dir = 0; dir < 4; dir++) {
			if (!HasWall(maze, i % maze.width, i / maze.width, static_cast<Direction>(dir))) {
				openings++;
			}
		}

		if (openings == 1 && !isOnPath[i]) {
			deadEnds[deadEndCount++] = i;
		}
	}

	// Shuffle the dead ends so the gems are in random places
	for (int i = deadEndCount - 1; i > 0; i--) {
		int j = random() % (i + 1);
		int temp = deadEnds[i];
		deadEnds[i] = deadEnds[j];
		deadEnds[j] = temp;
	}

	for (int i = 0; i < deadEndCount && (fireGems < gemsPerHero || waterGems < gemsPerHero); i++) {
		// Go back from the dead end until the next cell is on the way to the exit
		int entrance = deadEnds[i];

		while (!isOnPath[parent[entrance]]) {
			entrance = parent[entrance];
		}

		// Skip side ways that are too short or already have a gem
		if (entrance == deadEnds[i] || session.cells[entrance] != CELL_EMPTY) {
			continue;
		}

		bool isFireGem = fireGems <= waterGems && fireGems < gemsPerHero;

		session.cells[entrance] = isFireGem ? CELL_LAVA : CELL_WATER;
		session.cells[deadEnds[i]] = isFireGem ? CELL_FIRE_GEM : CELL_WATER_GEM;

		if (isFireGem) fireGems++;
		else waterGems++;
	}

	session.gemsLeft = fireGems + waterGems;
	delete[] deadEnds;
}

static void PlaceObjects(TeamSession& session, unsigned int seed) {
	const Maze& maze = session.maze;
	int cellCount = maze.width * maze.height;
	std::mt19937 random(seed);

	int* parent = new int[cellCount];
	int* path = new int[cellCount];
	bool* isOnPath = new bool[cellCount];

	FindParents(maze, parent);

	// The way from the exit back to the start (the maze is perfect, so there is only one)
	int pathLength = 0;

	for (int i = 0; i < cellCount; i++) {
		isOnPath[i] = false;
	}

	for (int cell = GetCellIndex(maze, GetExitX(maze), GetExitY(maze)); cell != -1; cell = parent[cell]) {
		path[pathLength++] = cell;
		isOnPath[cell] = true;
	}

	PlaceGates(session, path, pathLength);
	PlaceSpikes(session, path, pathLength, random);
	PlaceGems(session, parent, isOnPath, random);

	delete[] parent;
	delete[] path;
	delete[] isOnPath;
}

static void SendBackToStart(TeamSession& session, Hero hero) {
	session.heroes[hero].x = 0;
	session.heroes[hero].y = 0;
	session.lastHurtHero = hero;
	session.hurtTimer = HURT_MESSAGE_TIME;
}

// Lava hurts the water hero, water hurts the fire hero, raised spikes hurt both
static void CheckHazards(TeamSession& session, Hero hero) {
	const Player& player = session.heroes[hero];
	CellType type = GetCellType(session, player.x, player.y);

	bool isHurt = (type == CELL_LAVA && hero == HERO_WATER)
		|| (type == CELL_WATER && hero == HERO_FIRE)
		|| (type == CELL_SPIKES && AreSpikesUp(session, player.x, player.y));

	if (isHurt) {
		SendBackToStart(session, hero);
	}
}

static void CollectGem(TeamSession& session, Hero hero) {
	const Player& player = session.heroes[hero];
	CellType& cell = session.cells[GetCellIndex(session.maze, player.x, player.y)];

	if ((cell == CELL_FIRE_GEM && hero == HERO_FIRE) || (cell == CELL_WATER_GEM && hero == HERO_WATER)) {
		cell = CELL_EMPTY;
		session.gemsLeft--;
	}
}

// The team escapes when the exit is open and both heroes stand on it
static bool HaveBothEscaped(const TeamSession& session) {
	int exitX = GetExitX(session.maze);
	int exitY = GetExitY(session.maze);

	return IsExitOpen(session)
		&& session.heroes[HERO_FIRE].x == exitX && session.heroes[HERO_FIRE].y == exitY
		&& session.heroes[HERO_WATER].x == exitX && session.heroes[HERO_WATER].y == exitY;
}

bool StartTeamSession(TeamSession& session, int mazeSize, unsigned int seed) {
	if (!CreateMaze(session.maze, mazeSize, mazeSize)) {
		return false;
	}

	GenerateMaze(session.maze, seed);

	int cellCount = mazeSize * mazeSize;
	delete[] session.cells;
	session.cells = new CellType[cellCount];

	for (int i = 0; i < cellCount; i++) {
		session.cells[i] = CELL_EMPTY;
	}

	PlaceObjects(session, seed);

	session.heroes[HERO_FIRE] = Player{};
	session.heroes[HERO_WATER] = Player{};
	session.elapsedSeconds = 0.0f;
	session.isFinished = false;
	session.isPaused = false;
	session.hurtTimer = 0.0f;
	session.remoteHero = -1;

	return true;
}

bool RestartTeamSession(TeamSession& session, unsigned int seed) {
	return StartTeamSession(session, session.maze.width, seed);
}

void EndTeamSession(TeamSession& session) {
	DestroyMaze(session.maze);
	delete[] session.cells;
	session.cells = nullptr;
	session.isFinished = false;
	session.isPaused = false;
}

void SetTeamPaused(TeamSession& session, bool isPaused) {
	session.isPaused = isPaused && !session.isFinished;
}

bool TryMoveHero(TeamSession& session, Hero hero, Direction direction) {
	if (session.isFinished || session.isPaused) {
		return false;
	}

	Player& player = session.heroes[hero];
	player.facing = direction;

	if (HasWall(session.maze, player.x, player.y, direction)) {
		return false;
	}

	int nextX = player.x + DIRECTION_DX[direction];
	int nextY = player.y + DIRECTION_DY[direction];

	if (GetCellType(session, nextX, nextY) == CELL_GATE && !AreGatesOpen(session)) {
		return false;
	}

	player.x = nextX;
	player.y = nextY;
	player.moves++;

	CollectGem(session, hero);
	CheckHazards(session, hero);
	session.isFinished = HaveBothEscaped(session);

	return true;
}

CellType GetCellType(const TeamSession& session, int x, int y) {
	if (session.cells == nullptr || !IsInsideMaze(session.maze, x, y)) {
		return CELL_EMPTY;
	}

	return session.cells[GetCellIndex(session.maze, x, y)];
}

bool AreGatesOpen(const TeamSession& session) {
	for (int hero = 0; hero < HERO_COUNT; hero++) {
		if (GetCellType(session, session.heroes[hero].x, session.heroes[hero].y) == CELL_PLATE) {
			return true;
		}
	}

	return false;
}

bool AreSpikesUp(const TeamSession& session, int x, int y) {
	int phase = (x + y) % 2;
	return (static_cast<int>(session.elapsedSeconds) + phase) % 2 == 0;
}

bool IsExitOpen(const TeamSession& session) {
	return session.gemsLeft == 0;
}

void UpdateTeamGame(TeamSession& session, float deltaSeconds) {
	if (session.isFinished || session.isPaused) {
		return;
	}

	session.elapsedSeconds += deltaSeconds;

	if (session.hurtTimer > 0.0f) {
		session.hurtTimer -= deltaSeconds;
	}

	// The spikes can go up while a hero is standing on them.
	// In LAN the other computer checks its own hero, so a small time difference can't hurt it here
	for (int hero = 0; hero < HERO_COUNT; hero++) {
		if (hero != session.remoteHero) {
			CheckHazards(session, static_cast<Hero>(hero));
		}
	}
}

void SetRemoteHero(TeamSession& session, int hero) {
	session.remoteHero = (hero >= 0 && hero < HERO_COUNT) ? hero : -1;
}

void ApplyRemoteHero(TeamSession& session, Hero hero, int x, int y, int moves, Direction facing) {
	if (session.isFinished || !IsInsideMaze(session.maze, x, y)) {
		return;
	}

	Player& player = session.heroes[hero];
	player.x = x;
	player.y = y;
	player.moves = moves;
	player.facing = facing;

	CollectGem(session, hero);
	session.isFinished = HaveBothEscaped(session);
}

int GetTeamMoves(const TeamSession& session) {
	return session.heroes[HERO_FIRE].moves + session.heroes[HERO_WATER].moves;
}

TeamSession& GetCurrentTeamSession() {
	return currentTeamSession;
}
