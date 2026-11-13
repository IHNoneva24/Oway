#pragma once
#include "Game.h"

// Heroes
enum Hero {
	HERO_FIRE,
	HERO_WATER,
	HERO_COUNT
};

// Cell types
enum CellType : unsigned char {
	CELL_EMPTY,
	CELL_LAVA,
	CELL_WATER,
	CELL_SPIKES,
	CELL_PLATE,
	CELL_GATE,
	CELL_FIRE_GEM,
	CELL_WATER_GEM
};

const float HURT_MESSAGE_TIME = 2.0f;

// Team state
struct TeamSession {
	Maze maze;
	CellType* cells = nullptr;
	Player heroes[HERO_COUNT];
	int gemsLeft = 0;
	float elapsedSeconds = 0.0f;
	bool isFinished = false;
	bool isPaused = false;
	int remoteHero = -1;
	Hero lastHurtHero = HERO_FIRE;
	float hurtTimer = 0.0f;
};

// Start game
bool StartTeamSession(TeamSession& session, int mazeSize, unsigned int seed);

// Restart game
bool RestartTeamSession(TeamSession& session, unsigned int seed);

// End game
void EndTeamSession(TeamSession& session);

// Pause game
void SetTeamPaused(TeamSession& session, bool isPaused);

// Move hero
bool TryMoveHero(TeamSession& session, Hero hero, Direction direction);

// Cell type
CellType GetCellType(const TeamSession& session, int x, int y);

// Gates open?
bool AreGatesOpen(const TeamSession& session);

// Spikes up?
bool AreSpikesUp(const TeamSession& session, int x, int y);

// Exit open?
bool IsExitOpen(const TeamSession& session);

// Remote hero
void SetRemoteHero(TeamSession& session, int hero);

// Remote move
void ApplyRemoteHero(TeamSession& session, Hero hero, int x, int y, int moves, Direction facing);

// Update game
void UpdateTeamGame(TeamSession& session, float deltaSeconds);

// Team moves
int GetTeamMoves(const TeamSession& session);

// Current game
TeamSession& GetCurrentTeamSession();
