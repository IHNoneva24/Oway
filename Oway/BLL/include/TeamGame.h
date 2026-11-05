#pragma once
#include "Game.h"

// Team mode logic - two heroes on the same maze who have to escape together

enum Hero {
	HERO_FIRE,
	HERO_WATER,
	HERO_COUNT
};

// What is on a cell of the maze (one value per cell)
enum CellType : unsigned char {
	CELL_EMPTY,
	CELL_LAVA,        // only the fire hero can walk on it
	CELL_WATER,       // only the water hero can walk on it
	CELL_SPIKES,      // go up and down every second, dangerous for both
	CELL_PLATE,       // while a hero stands on any plate all gates are open
	CELL_GATE,        // blocks the way while it is closed
	CELL_FIRE_GEM,    // collected by the fire hero
	CELL_WATER_GEM    // collected by the water hero
};

// How long the "sent back" message is shown (in seconds)
const float HURT_MESSAGE_TIME = 2.0f;

struct TeamSession {
	Maze maze;
	CellType* cells = nullptr; // dynamic array of width * height cell types
	Player heroes[HERO_COUNT];
	int gemsLeft = 0;
	float elapsedSeconds = 0.0f;
	bool isFinished = false;
	bool isPaused = false;
	Hero lastHurtHero = HERO_FIRE;
	float hurtTimer = 0.0f; // > 0 while the "sent back" message is shown
};

// Creates a new maze with traps, gates and gems. Both heroes start at 0,0
bool StartTeamSession(TeamSession& session, int mazeSize, unsigned int seed);

// Generates a new maze with the same size and starts again
bool RestartTeamSession(TeamSession& session, unsigned int seed);

void EndTeamSession(TeamSession& session);

void SetTeamPaused(TeamSession& session, bool isPaused);

// Moves one hero a cell if there is no wall or closed gate. Returns true if the hero moved
bool TryMoveHero(TeamSession& session, Hero hero, Direction direction);

CellType GetCellType(const TeamSession& session, int x, int y);

// The gates are open while at least one hero stands on a plate
bool AreGatesOpen(const TeamSession& session);

// The spikes on neighbouring cells go up and down at different times
bool AreSpikesUp(const TeamSession& session, int x, int y);

// The exit opens when all gems are collected
bool IsExitOpen(const TeamSession& session);

// Adds the frame time, checks the spikes and the win condition
void UpdateTeamGame(TeamSession& session, float deltaSeconds);

int GetTeamMoves(const TeamSession& session);

// The single team session shared between the screens
TeamSession& GetCurrentTeamSession();
