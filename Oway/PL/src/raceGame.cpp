#include "screens.h"
#include "ui.h"
#include "assets.h"
#include "mazeView.h"
#include "RaceGame.h"
#include "LanGame.h"
#include "lanSession.h"
#include <ctime>

// from screen.cpp
extern int screenW;
extern int screenH;
// from screen.cpp
extern bool isLanGame;

static const float VIEW_MARGIN = 20.0f;

static const int RACER_SPRITE_COLUMNS[RACER_COUNT] = { 0, 3 };
static const char* RACER_KEYS[RACER_COUNT] = { "WASD", "Arrows" };

static const int RACER_KEY_CODES[RACER_COUNT][4] = {
	{ KEY_W, KEY_D, KEY_S, KEY_A },
	{ KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_LEFT }
};

// Movement keys
static void HandleMovement(RaceSession& session) {
	if (isLanGame) {
		int local = GetLocalPlayer();

		for (int dir = 0; dir < 4; dir++) {
			for (int keys = 0; keys < RACER_COUNT; keys++) {
				int key = RACER_KEY_CODES[keys][dir];

				if (IsKeyPressed(key) || IsKeyPressedRepeat(key)) {
					TryMoveRacer(session, local, static_cast<Direction>(dir));
				}
			}
		}

		return;
	}

	for (int racer = 0; racer < RACER_COUNT; racer++) {
		for (int dir = 0; dir < 4; dir++) {
			int key = RACER_KEY_CODES[racer][dir];

			if (IsKeyPressed(key) || IsKeyPressedRepeat(key)) {
				TryMoveRacer(session, racer, static_cast<Direction>(dir));
			}
		}
	}
}

// Racer view
static void DrawRacerView(const RaceSession& session, int racer, Rectangle area) {
	const Player& player = session.racers[racer];
	bool isOut = session.hasEscaped[racer];

	float labelSize = Scaled(30);
	const char* who = !isLanGame ? RACER_KEYS[racer] : (racer == GetLocalPlayer() ? "you" : "LAN");
	const char* label = TextFormat("Player %d (%s)  -  %d moves", racer + 1, who, player.moves);
	int labelWidth = MeasureText(label, static_cast<int>(labelSize));

	DrawText(label, static_cast<int>(area.x + area.width / 2 - labelWidth / 2), static_cast<int>(area.y),
		static_cast<int>(labelSize), isOut ? COLOR_ACCENT : RAYWHITE);

	area.y += labelSize + Scaled(10);
	area.height -= labelSize + Scaled(10);

	const Maze& maze = session.mazes[racer];
	DrawMazeWallsIn(maze, area);
	DrawExitIn(maze, area, true);

	// idle animation
	int frameRow = static_cast<int>(GetTime() * 2) % 2 == 0 ? 0 : 2;
	Rectangle playerTile = GetCellRectIn(maze, area, player.x, player.y);
	DrawTile(GetAssets().characters, RACER_SPRITE_COLUMNS[racer], frameRow, playerTile, player.facing == DIRECTION_LEFT);

	if (isOut && !session.isFinished) {
		const char* message = TextFormat("ESCAPED in %.1f s", session.escapeSeconds[racer]);
		int fontSize = static_cast<int>(Scaled(48));

		DrawRectangleRec(area, { 0, 0, 0, 150 });
		DrawText(message, static_cast<int>(area.x + area.width / 2 - MeasureText(message, fontSize) / 2),
			static_cast<int>(area.y + area.height / 2 - fontSize / 2.0f), fontSize, COLOR_ACCENT);
	}
}

// Draw HUD
static void DrawRaceHud(const RaceSession& session) {
	float height = Scaled(HUD_HEIGHT);
	int fontSize = static_cast<int>(Scaled(30));
	int textY = static_cast<int>(height / 2 - fontSize / 2.0f);

	DrawRectangle(0, 0, screenW, static_cast<int>(height), COLOR_PANEL);

	DrawText(TextFormat("Maze: %d x %d", session.mazes[0].width, session.mazes[0].height),
		static_cast<int>(Scaled(30)), textY, fontSize, RAYWHITE);
	DrawText(TextFormat("Time: %.1f s", session.elapsedSeconds), static_cast<int>(Scaled(330)), textY, fontSize, RAYWHITE);
	DrawCenteredText("Race to the ladder!", static_cast<float>(textY), static_cast<float>(fontSize), COLOR_ACCENT);

	const char* hint = "ESC - pause";
	DrawText(hint, screenW - MeasureText(hint, fontSize) - static_cast<int>(Scaled(30)), textY, fontSize, COLOR_TEXT_DIM);
}

// Pause menu
static void DrawRacePauseMenu(RaceSession& session, Screen& currentScreen) {
	DrawRectangle(0, 0, screenW, screenH, { 0, 0, 0, 170 });

	float y = screenH * 0.3f;
	DrawCenteredText("PAUSED", y, Scaled(80), COLOR_ACCENT);

	y += Scaled(140);

	if (DrawCenteredButton(y, "Resume")) {
		SetRacePaused(session, false);
	}

	if (DrawCenteredButton(y + Scaled(95), "Restart (new maze)")) {
		RestartRaceSession(session, static_cast<unsigned int>(time(nullptr)));
	}

	if (DrawCenteredButton(y + Scaled(190), "Main menu")) {
		EndRaceSession(session);
		currentScreen = MAINMENU;
	}
}

// LAN sync
static void SyncLanRace(RaceSession& session, Screen& currentScreen) {
	int local = GetLocalPlayer();
	SendLocalPlayer(session.racers[local]);

	if (session.hasEscaped[local]) {
		SendLocalEscape(session.escapeSeconds[local]);
	}

	UpdateLanSession(currentScreen);

	const RemotePlayer& remote = GetRemotePlayer();
	ApplyRemoteRacer(session, 1 - local, remote.x, remote.y, remote.moves, remote.facing,
		remote.hasEscaped, remote.escapeSeconds);
}

// Race screen
void DrawRaceGame(Screen& currentScreen) {
	RaceSession& session = GetCurrentRaceSession();

	if (IsKeyPressed(KEY_ESCAPE)) {
		if (isLanGame) {
			ToggleLanLeaveMenu();
		}
		else {
			SetRacePaused(session, !session.isPaused);
		}
	}

	HandleMovement(session);
	UpdateRace(session, GetFrameTime());

	if (isLanGame) {
		SyncLanRace(session, currentScreen);

		if (currentScreen != RACEGAME) {
			return;
		}
	}

	ClearBackground(COLOR_BACKGROUND);

	float margin = Scaled(VIEW_MARGIN);
	float top = Scaled(HUD_HEIGHT) + margin;
	float height = screenH - top - margin;
	float halfWidth = screenW / 2.0f;

	DrawRacerView(session, 0, { margin, top, halfWidth - margin * 1.5f, height });
	DrawRacerView(session, 1, { halfWidth + margin * 0.5f, top, halfWidth - margin * 1.5f, height });
	DrawRectangle(static_cast<int>(halfWidth - Scaled(2)), static_cast<int>(Scaled(HUD_HEIGHT)),
		static_cast<int>(Scaled(4)), screenH, { 92, 60, 52, 255 });

	DrawRaceHud(session);

	if (session.isPaused) {
		DrawRacePauseMenu(session, currentScreen);
	}

	if (isLanGame && (DrawLanNotice(currentScreen) || DrawLanLeaveMenu(currentScreen))) {
		return;
	}

	if (session.isFinished) {
		currentScreen = RACEWIN;
	}
}
