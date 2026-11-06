#pragma once
#include "Game.h"

// Race mode logic - two players, each in an own copy of the same maze, who is out first wins

const int RACER_COUNT = 2;

struct RaceSession {
	Maze mazes[RACER_COUNT]; // same seed, so both mazes have the same layout
	Player racers[RACER_COUNT];
	bool hasEscaped[RACER_COUNT] = { false, false };
	float escapeSeconds[RACER_COUNT] = { 0.0f, 0.0f };
	float elapsedSeconds = 0.0f;
	bool isFinished = false; // true when both racers are out
	bool isPaused = false;
};

// Creates both mazes from the same seed and places both racers at 0,0
bool StartRaceSession(RaceSession& session, int mazeSize, unsigned int seed);

// Generates new mazes with the same size and starts again
bool RestartRaceSession(RaceSession& session, unsigned int seed);

void EndRaceSession(RaceSession& session);

void SetRacePaused(RaceSession& session, bool isPaused);

// Moves one racer a cell in the racer's own maze. Returns true if the racer moved
bool TryMoveRacer(RaceSession& session, int racer, Direction direction);

// LAN: puts the other player where the other computer says. Positions outside the maze are ignored
void ApplyRemoteRacer(RaceSession& session, int racer, int x, int y, int moves, Direction facing,
	bool hasEscaped, float escapeSeconds);

// Adds the frame time while the race is running
void UpdateRace(RaceSession& session, float deltaSeconds);

// Index of the racer who escaped first, or -1 if nobody is out yet
int GetRaceWinner(const RaceSession& session);

// The single race session shared between the screens
RaceSession& GetCurrentRaceSession();
