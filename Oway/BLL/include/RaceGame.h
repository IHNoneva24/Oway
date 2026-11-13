#pragma once
#include "Game.h"

const int RACER_COUNT = 2;

// Race state
struct RaceSession {
	Maze mazes[RACER_COUNT];
	Player racers[RACER_COUNT];
	bool hasEscaped[RACER_COUNT] = { false, false };
	float escapeSeconds[RACER_COUNT] = { 0.0f, 0.0f };
	float elapsedSeconds = 0.0f;
	bool isFinished = false;
	bool isPaused = false;
};

// Start race
bool StartRaceSession(RaceSession& session, int mazeSize, unsigned int seed);

// Restart race
bool RestartRaceSession(RaceSession& session, unsigned int seed);

// End race
void EndRaceSession(RaceSession& session);

// Pause race
void SetRacePaused(RaceSession& session, bool isPaused);

// Move racer
bool TryMoveRacer(RaceSession& session, int racer, Direction direction);

// Remote racer
void ApplyRemoteRacer(RaceSession& session, int racer, int x, int y, int moves, Direction facing,
	bool hasEscaped, float escapeSeconds);

// Update timer
void UpdateRace(RaceSession& session, float deltaSeconds);

// Race winner
int GetRaceWinner(const RaceSession& session);

// Current race
RaceSession& GetCurrentRaceSession();
