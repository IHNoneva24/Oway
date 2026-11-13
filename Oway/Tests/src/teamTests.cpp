#include "tests.h"
#include "testFramework.h"
#include "testHelpers.h"
#include "TeamGame.h"

// Find cell
static bool FindCell(const TeamSession& session, CellType type, int& x, int& y) {
	for (y = 0; y < session.maze.height; y++) {
		for (x = 0; x < session.maze.width; x++) {
			if (GetCellType(session, x, y) == type) {
				return true;
			}
		}
	}

	return false;
}

// Place hero
static void PlaceHero(TeamSession& session, Hero hero, int x, int y) {
	session.heroes[hero].x = x;
	session.heroes[hero].y = y;
}

// Starting
static void TestStart() {
	TeamSession session;

	Check(!StartTeamSession(session, 70, 1u), "a team game with size 70 doesn't start");
	Check(StartTeamSession(session, 30, 21u), "a team game with size 30 starts");
	Check(session.cells != nullptr, "the cell types array is allocated");
	Check(session.gemsLeft > 0, "the maze has gems to collect");
	Check(!IsExitOpen(session), "the exit is closed while there are gems left");
	Check(GetCellType(session, 0, 0) == CELL_EMPTY, "the start cell has no trap");
	Check(GetCellType(session, 29, 29) == CELL_EMPTY, "the exit cell has no trap");

	int x = 0;
	int y = 0;
	Check(FindCell(session, CELL_GATE, x, y), "a 30x30 maze has gates");
	Check(FindCell(session, CELL_PLATE, x, y), "a 30x30 maze has plates");

	EndTeamSession(session);
	Check(session.cells == nullptr && session.maze.cells == nullptr, "EndTeamSession frees both arrays");
}

// Lava, water
static void TestPools() {
	TeamSession session;
	StartTeamSession(session, 30, 21u);

	int lavaX = 0;
	int lavaY = 0;
	int fromX = 0;
	int fromY = 0;
	Direction towardsLava = DIRECTION_UP;

	if (!FindCell(session, CELL_LAVA, lavaX, lavaY) || !FindOpenNeighbour(session.maze, lavaX, lavaY, fromX, fromY, towardsLava)) {
		Check(false, "the test maze has a lava cell");
		EndTeamSession(session);
		return;
	}

	PlaceHero(session, HERO_WATER, fromX, fromY);
	TryMoveHero(session, HERO_WATER, towardsLava);
	Check(session.heroes[HERO_WATER].x == 0 && session.heroes[HERO_WATER].y == 0, "lava sends the water hero back to the start");
	Check(session.hurtTimer > 0.0f && session.lastHurtHero == HERO_WATER, "the \"sent back\" message is shown for the water hero");

	PlaceHero(session, HERO_FIRE, fromX, fromY);
	TryMoveHero(session, HERO_FIRE, towardsLava);
	Check(session.heroes[HERO_FIRE].x == lavaX && session.heroes[HERO_FIRE].y == lavaY, "the fire hero can stand on lava");

	EndTeamSession(session);
}

// Gates
static void TestGates() {
	TeamSession session;
	StartTeamSession(session, 30, 21u);

	int gateX = 0;
	int gateY = 0;
	int fromX = 0;
	int fromY = 0;
	int plateX = 0;
	int plateY = 0;
	Direction towardsGate = DIRECTION_UP;

	if (!FindCell(session, CELL_GATE, gateX, gateY) || !FindOpenNeighbour(session.maze, gateX, gateY, fromX, fromY, towardsGate)
		|| !FindCell(session, CELL_PLATE, plateX, plateY)) {
		Check(false, "the test maze has a gate and a plate");
		EndTeamSession(session);
		return;
	}

	PlaceHero(session, HERO_FIRE, fromX, fromY);
	PlaceHero(session, HERO_WATER, 0, 0);
	Check(!AreGatesOpen(session), "the gates are closed while nobody stands on a plate");
	Check(!TryMoveHero(session, HERO_FIRE, towardsGate), "a closed gate blocks the way");

	PlaceHero(session, HERO_WATER, plateX, plateY);
	Check(AreGatesOpen(session), "the gates open while a hero stands on a plate");
	Check(TryMoveHero(session, HERO_FIRE, towardsGate), "the other hero can walk through the open gate");

	EndTeamSession(session);
}

// Gems
static void TestGems() {
	TeamSession session;
	StartTeamSession(session, 30, 21u);

	int gemX = 0;
	int gemY = 0;
	int fromX = 0;
	int fromY = 0;
	Direction towardsGem = DIRECTION_UP;

	if (!FindCell(session, CELL_FIRE_GEM, gemX, gemY) || !FindOpenNeighbour(session.maze, gemX, gemY, fromX, fromY, towardsGem)) {
		Check(false, "the test maze has a fire gem");
		EndTeamSession(session);
		return;
	}

	int gemsBefore = session.gemsLeft;

	PlaceHero(session, HERO_WATER, fromX, fromY);
	TryMoveHero(session, HERO_WATER, towardsGem);
	CheckEqual(gemsBefore, session.gemsLeft, "the water hero can't take a fire gem");

	PlaceHero(session, HERO_FIRE, fromX, fromY);
	TryMoveHero(session, HERO_FIRE, towardsGem);
	CheckEqual(gemsBefore - 1, session.gemsLeft, "the fire hero takes the fire gem");
	Check(GetCellType(session, gemX, gemY) == CELL_EMPTY, "a taken gem disappears from the maze");

	EndTeamSession(session);
}

// Spikes
static void TestSpikes() {
	bool hasNoSpikePairs = true;

	for (unsigned int seed = 1; seed <= 300; seed++) {
		TeamSession session;
		StartTeamSession(session, MIN_MAZE_SIZE + seed % 41, seed);

		for (int y = 0; y < session.maze.height; y++) {
			for (int x = 0; x < session.maze.width; x++) {
				if (GetCellType(session, x, y) != CELL_SPIKES) {
					continue;
				}

				hasNoSpikePairs = hasNoSpikePairs
					&& !(!HasWall(session.maze, x, y, DIRECTION_RIGHT) && GetCellType(session, x + 1, y) == CELL_SPIKES)
					&& !(!HasWall(session.maze, x, y, DIRECTION_DOWN) && GetCellType(session, x, y + 1) == CELL_SPIKES);
			}
		}

		EndTeamSession(session);
	}

	Check(hasNoSpikePairs, "there are never two connected spikes next to each other (300 mazes)");

	TeamSession session;
	StartTeamSession(session, 30, 21u);

	session.elapsedSeconds = 0.0f;
	bool isUpFirst = AreSpikesUp(session, 3, 3);
	session.elapsedSeconds = 1.0f;
	Check(AreSpikesUp(session, 3, 3) != isUpFirst, "the spikes go up and down every second");
	Check(AreSpikesUp(session, 3, 3) != AreSpikesUp(session, 4, 3), "neighbouring spikes move at opposite times");

	EndTeamSession(session);
}

// Remote hero
static void TestRemoteHero() {
	TeamSession session;
	StartTeamSession(session, 30, 21u);

	int spikeX = 0;
	int spikeY = 0;

	if (!FindCell(session, CELL_SPIKES, spikeX, spikeY)) {
		Check(false, "the test maze has spikes");
		EndTeamSession(session);
		return;
	}

	session.elapsedSeconds = AreSpikesUp(session, spikeX, spikeY) ? 0.0f : 1.0f;

	SetRemoteHero(session, HERO_WATER);
	PlaceHero(session, HERO_WATER, spikeX, spikeY);
	UpdateTeamGame(session, 0.0f);
	Check(session.heroes[HERO_WATER].x == spikeX, "the other computer's hero is not sent back here");

	SetRemoteHero(session, -1);
	UpdateTeamGame(session, 0.0f);
	Check(session.heroes[HERO_WATER].x == 0 && session.heroes[HERO_WATER].y == 0, "raised spikes send a local hero back");

	ApplyRemoteHero(session, HERO_FIRE, -4, 100, 3, DIRECTION_UP);
	Check(session.heroes[HERO_FIRE].x != -4, "a position outside the maze from the network is ignored");

	EndTeamSession(session);
}

// Escaping
static void TestEscape() {
	TeamSession session;
	StartTeamSession(session, 20, 5u);

	ApplyRemoteHero(session, HERO_FIRE, 19, 19, 50, DIRECTION_DOWN);
	ApplyRemoteHero(session, HERO_WATER, 19, 19, 60, DIRECTION_DOWN);
	Check(!session.isFinished, "the heroes can't escape before all gems are taken");

	session.gemsLeft = 0;
	Check(IsExitOpen(session), "the exit opens when all gems are taken");

	ApplyRemoteHero(session, HERO_WATER, 19, 19, 61, DIRECTION_DOWN);
	Check(session.isFinished, "the team escapes when both heroes are on the open exit");
	CheckEqual(111, GetTeamMoves(session), "the team moves are the moves of both heroes together");

	EndTeamSession(session);
}

// Team tests
void RunTeamTests() {
	BeginTestGroup("Team play (BLL/TeamGame)");

	TestStart();
	TestPools();
	TestGates();
	TestGems();
	TestSpikes();
	TestRemoteHero();
	TestEscape();
}
