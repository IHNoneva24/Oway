#include "screens.h"
#include "ui.h"
#include "assets.h"
#include "mazeView.h"
#include "TeamGame.h"
#include <ctime>
#include <cmath>

extern int screenW;
extern int screenH;

// A held key moves the hero again after these delays (in seconds).
// Every hero has its own timer, so both players can hold their keys at the same time
static const float FIRST_REPEAT_DELAY = 0.25f;
static const float REPEAT_DELAY = 0.1f;

static float moveTimers[HERO_COUNT] = { 0.0f, 0.0f };

// Tiles from Dungeon_Tileset.png (column, row)
static const int GATE_TILE[2] = { 8, 3 };
static const int FIRE_GEM_TILE[2] = { 9, 8 };
static const int WATER_GEM_TILE[2] = { 7, 8 };

// Columns of the heroes in Dungeon_Character.png
static const int FIRE_HERO_COLUMN = 1;
static const int WATER_HERO_COLUMN = 3;

static const Color COLOR_FIRE = { 240, 110, 50, 255 };
static const Color COLOR_WATER = { 70, 160, 240, 255 };

// Moves the hero when the key is pressed, or again while it is held down
static void HandleHeroKey(TeamSession& session, Hero hero, int key, Direction direction) {
	if (IsKeyPressed(key)) {
		TryMoveHero(session, hero, direction);
		moveTimers[hero] = FIRST_REPEAT_DELAY;
	}
	else if (IsKeyDown(key) && moveTimers[hero] <= 0.0f) {
		TryMoveHero(session, hero, direction);
		moveTimers[hero] = REPEAT_DELAY;
	}
}

// Fire hero - WASD, water hero - arrow keys
static void HandleMovement(TeamSession& session) {
	moveTimers[HERO_FIRE] -= GetFrameTime();
	moveTimers[HERO_WATER] -= GetFrameTime();

	HandleHeroKey(session, HERO_FIRE, KEY_W, DIRECTION_UP);
	HandleHeroKey(session, HERO_FIRE, KEY_D, DIRECTION_RIGHT);
	HandleHeroKey(session, HERO_FIRE, KEY_S, DIRECTION_DOWN);
	HandleHeroKey(session, HERO_FIRE, KEY_A, DIRECTION_LEFT);

	HandleHeroKey(session, HERO_WATER, KEY_UP, DIRECTION_UP);
	HandleHeroKey(session, HERO_WATER, KEY_RIGHT, DIRECTION_RIGHT);
	HandleHeroKey(session, HERO_WATER, KEY_DOWN, DIRECTION_DOWN);
	HandleHeroKey(session, HERO_WATER, KEY_LEFT, DIRECTION_LEFT);
}

// Draws a whole texture stretched into the destination
static void DrawTextureInRect(Texture2D texture, Rectangle destination) {
	DrawTexturePro(texture, { 0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height) },
		destination, { 0, 0 }, 0.0f, WHITE);
}

// Draws the lava, water, spikes, plates, gates and gems of one cell
static void DrawCellObject(const TeamSession& session, int x, int y) {
	const GameAssets& assets = GetAssets();
	Rectangle cell = GetCellRect(session.maze, x, y);
	float wave = sinf(static_cast<float>(GetTime()) * 3 + x + y);

	switch (GetCellType(session, x, y)) {
	case CELL_LAVA:
		// The flame sprite is 16x32, so it is drawn half as wide as the cell. Its last frame has no flame
		DrawRectangleRec(cell, { 230, 80, 30, static_cast<unsigned char>(180 + 40 * wave) });
		DrawTextureInRect(assets.flame[GetAnimationFrame() % 3], { cell.x + cell.width / 4, cell.y, cell.width / 2, cell.height });
		break;

	case CELL_WATER:
		DrawRectangleRec(cell, { 40, 110, 230, 220 });
		DrawRectangleRec({ cell.x, cell.y + cell.height * (0.4f + 0.2f * wave), cell.width, cell.height * 0.12f }, { 160, 215, 255, 200 });
		break;

	case CELL_SPIKES:
		DrawTextureInRect(AreSpikesUp(session, x, y) ? assets.spikes[0] : assets.spikes[3], cell);
		break;

	case CELL_PLATE: {
		float inset = cell.width * 0.15f;
		Rectangle plate = { cell.x + inset, cell.y + inset, cell.width - inset * 2, cell.height - inset * 2 };
		DrawRectangleRec(plate, AreGatesOpen(session) ? COLOR_ACCENT : Color{ 120, 110, 120, 255 });
		DrawRectangleLinesEx(plate, cell.width * 0.08f, { 50, 30, 30, 255 });
		break;
	}

	case CELL_GATE:
		if (AreGatesOpen(session)) {
			DrawRectangleLinesEx(cell, cell.width * 0.08f, { 240, 180, 60, 120 });
		}
		else {
			DrawTile(assets.tileset, GATE_TILE[0], GATE_TILE[1], cell);
		}
		break;

	case CELL_FIRE_GEM:
		DrawTile(assets.tileset, FIRE_GEM_TILE[0], FIRE_GEM_TILE[1], cell);
		break;

	case CELL_WATER_GEM:
		DrawTile(assets.tileset, WATER_GEM_TILE[0], WATER_GEM_TILE[1], cell);
		break;

	case CELL_EMPTY:
		break;
	}
}

// Draws a hero with a colored glow under it. "shift" moves it sideways when both heroes share a cell
static void DrawHero(const TeamSession& session, Hero hero, int column, Color color, float shift) {
	const Player& player = session.heroes[hero];
	Rectangle tile = GetCellRect(session.maze, player.x, player.y);
	tile.x += tile.width * shift;

	DrawCircle(static_cast<int>(tile.x + tile.width / 2), static_cast<int>(tile.y + tile.height / 2),
		tile.width * 0.5f, Fade(color, 0.75f));

	// The hero blinks after it was sent back to the start
	bool isHurt = session.hurtTimer > 0.0f && session.lastHurtHero == hero;
	bool isVisible = !isHurt || static_cast<int>(GetTime() * 8) % 2 == 0;

	if (isVisible) {
		int frameRow = static_cast<int>(GetTime() * 2) % 2 == 0 ? 0 : 2;
		DrawTile(GetAssets().characters, column, frameRow, tile, player.facing == DIRECTION_LEFT);
	}
}

static void DrawTeamMaze(const TeamSession& session) {
	DrawMazeWalls(session.maze);
	DrawExit(session.maze, IsExitOpen(session));

	for (int y = 0; y < session.maze.height; y++) {
		for (int x = 0; x < session.maze.width; x++) {
			DrawCellObject(session, x, y);
		}
	}

	const Player& fire = session.heroes[HERO_FIRE];
	const Player& water = session.heroes[HERO_WATER];
	float shift = fire.x == water.x && fire.y == water.y ? 0.2f : 0.0f;

	DrawHero(session, HERO_FIRE, FIRE_HERO_COLUMN, COLOR_FIRE, -shift);
	DrawHero(session, HERO_WATER, WATER_HERO_COLUMN, COLOR_WATER, shift);
}

static void DrawTeamHud(const TeamSession& session) {
	float height = Scaled(HUD_HEIGHT);
	int fontSize = static_cast<int>(Scaled(30));
	int textY = static_cast<int>(height / 2 - fontSize / 2.0f);

	DrawRectangle(0, 0, screenW, static_cast<int>(height), COLOR_PANEL);

	DrawText(TextFormat("Maze: %d x %d", session.maze.width, session.maze.height), static_cast<int>(Scaled(30)), textY, fontSize, RAYWHITE);
	DrawText(TextFormat("Moves: %d", GetTeamMoves(session)), static_cast<int>(Scaled(330)), textY, fontSize, RAYWHITE);
	DrawText(TextFormat("Time: %.1f s", session.elapsedSeconds), static_cast<int>(Scaled(580)), textY, fontSize, RAYWHITE);

	if (IsExitOpen(session)) {
		DrawText("Exit open - go together!", static_cast<int>(Scaled(830)), textY, fontSize, COLOR_ACCENT);
	}
	else {
		DrawText(TextFormat("Gems left: %d", session.gemsLeft), static_cast<int>(Scaled(830)), textY, fontSize, RAYWHITE);
	}

	const char* hint = "ESC - pause";
	DrawText(hint, screenW - MeasureText(hint, fontSize) - static_cast<int>(Scaled(30)), textY, fontSize, COLOR_TEXT_DIM);

	// Message under the HUD after a hero stepped on the wrong thing
	if (session.hurtTimer > 0.0f) {
		bool isFire = session.lastHurtHero == HERO_FIRE;
		const char* message = isFire ? "Fire hero was sent back to the start!" : "Water hero was sent back to the start!";

		DrawRectangle(0, static_cast<int>(height), screenW, static_cast<int>(Scaled(50)), COLOR_PANEL);
		DrawCenteredText(message, height + Scaled(10), Scaled(30), isFire ? COLOR_FIRE : COLOR_WATER);
	}
}

static void DrawTeamPauseMenu(TeamSession& session, Screen& currentScreen) {
	DrawRectangle(0, 0, screenW, screenH, { 0, 0, 0, 170 });

	float y = screenH * 0.3f;
	DrawCenteredText("PAUSED", y, Scaled(80), COLOR_ACCENT);

	y += Scaled(140);

	if (DrawCenteredButton(y, "Resume")) {
		SetTeamPaused(session, false);
	}

	if (DrawCenteredButton(y + Scaled(95), "Restart (new maze)")) {
		RestartTeamSession(session, static_cast<unsigned int>(time(nullptr)));
	}

	if (DrawCenteredButton(y + Scaled(190), "Main menu")) {
		EndTeamSession(session);
		currentScreen = MAINMENU;
	}
}

void DrawTeamGame(Screen& currentScreen) {
	TeamSession& session = GetCurrentTeamSession();

	if (IsKeyPressed(KEY_ESCAPE)) {
		SetTeamPaused(session, !session.isPaused);
	}

	HandleMovement(session);
	UpdateTeamGame(session, GetFrameTime());

	ClearBackground(COLOR_BACKGROUND);
	DrawTeamMaze(session);
	DrawTeamHud(session);

	if (session.isPaused) {
		DrawTeamPauseMenu(session, currentScreen);
	}

	if (session.isFinished) {
		currentScreen = TEAMWIN;
	}
}
