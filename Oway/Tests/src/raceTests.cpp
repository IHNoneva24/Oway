#include "tests.h"
#include "testFramework.h"
#include "RaceGame.h"

// Same mazes
static void TestSameMazes() {
	RaceSession session;

	Check(!StartRaceSession(session, 60, 1u), "a race with size 60 doesn't start");
	Check(StartRaceSession(session, 18, 99u), "a race with size 18 starts");

	bool isSameLayout = true;

	for (int i = 0; i < 18 * 18; i++) {
		isSameLayout = isSameLayout && session.mazes[0].cells[i] == session.mazes[1].cells[i];
	}

	Check(isSameLayout, "both racers get the same maze layout (fair race)");
	Check(session.mazes[0].cells != session.mazes[1].cells, "each racer has an own copy of the maze");

	EndRaceSession(session);
}

// Own mazes
static void TestOwnMazes() {
	RaceSession session;
	StartRaceSession(session, 12, 5u);

	Direction open = HasWall(session.mazes[0], 0, 0, DIRECTION_RIGHT) ? DIRECTION_DOWN : DIRECTION_RIGHT;

	Check(TryMoveRacer(session, 0, open), "player 1 can move");
	CheckEqual(1, session.racers[0].moves, "player 1 has 1 move");
	CheckEqual(0, session.racers[1].moves, "player 2 didn't move (the racers move separately)");
	Check(!TryMoveRacer(session, 5, open), "a racer that doesn't exist can't move");

	EndRaceSession(session);
}

// Winner
static void TestWinner() {
	RaceSession session;
	StartRaceSession(session, 10, 8u);

	CheckEqual(-1, GetRaceWinner(session), "nobody wins before somebody escapes");

	ApplyRemoteRacer(session, 1, 9, 9, 40, DIRECTION_DOWN, true, 12.5f);
	CheckEqual(1, GetRaceWinner(session), "player 2 is the winner after escaping first");
	Check(!session.isFinished, "the race goes on until both racers are out");
	Check(!TryMoveRacer(session, 1, DIRECTION_UP), "a racer who is out can't move any more");

	ApplyRemoteRacer(session, 0, 9, 9, 35, DIRECTION_RIGHT, true, 20.0f);
	Check(session.isFinished, "the race ends when both racers are out");
	CheckEqual(1, GetRaceWinner(session), "the winner is the faster one, not the one with fewer moves");

	EndRaceSession(session);
}

// Remote racer
static void TestRemoteRacer() {
	RaceSession session;
	StartRaceSession(session, 10, 4u);

	ApplyRemoteRacer(session, 1, 50, -3, 7, DIRECTION_UP, false, 0.0f);
	Check(session.racers[1].x == 0 && session.racers[1].y == 0, "a position outside the maze from the network is ignored");

	ApplyRemoteRacer(session, 1, 3, 4, 7, DIRECTION_LEFT, false, 0.0f);
	Check(session.racers[1].x == 3 && session.racers[1].y == 4 && session.racers[1].moves == 7,
		"a correct position from the network is used");

	EndRaceSession(session);
}

// Race tests
void RunRaceTests() {
	BeginTestGroup("Race mode (BLL/RaceGame)");

	TestSameMazes();
	TestOwnMazes();
	TestWinner();
	TestRemoteRacer();
}
