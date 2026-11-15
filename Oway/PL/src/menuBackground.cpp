#include "menuBackground.h"
#include "ui.h"
#include "assets.h"
#include "mazeView.h"
#include <cmath>
#include <ctime>

// from screen.cpp
extern int screenW;
extern int screenH;

static const int EMBER_COUNT = 70;
static const int WALKER_COUNT = 3;
static const float RIPPLE_TIME = 0.9f;

// Floating spark
struct Ember {
	Vector2 position;
	float speed;
	float size;
	float phase;
};

// Character walking the maze
struct MazeWalker {
	int x;
	int y;
	int fromX;
	int fromY;
	float progress;
	float speed;
	bool isFacingLeft;
};

// columns in the character sheet
static const int WALKER_SPRITES[WALKER_COUNT] = { 0, 1, 3 };

static const Color WALKER_COLORS[WALKER_COUNT] = {
	{ 255, 190, 90, 255 },
	{ 255, 110, 50, 255 },
	{ 80, 170, 255, 255 }
};

static RenderTexture2D background;
static int backgroundW = 0;
static int backgroundH = 0;
static float padding = 0.0f;

static Maze maze;
static Rectangle mazeArea;

static Ember embers[EMBER_COUNT];
static MazeWalker walkers[WALKER_COUNT];

static Vector2 offset = { 0, 0 };
static Vector2 torch = { 0, 0 };
static Vector2 rippleCenter = { 0, 0 };
static float rippleAge = RIPPLE_TIME;

// Random number
static float RandomFloat(float low, float high) {
	return low + (high - low) * GetRandomValue(0, 1000) / 1000.0f;
}

// Cell center
static Vector2 GetCellCenter(int x, int y) {
	Rectangle cell = GetCellRectIn(maze, mazeArea, x, y);
	return { cell.x + cell.width / 2 - padding, cell.y + cell.height / 2 - padding };
}

// Walker position
static Vector2 GetWalkerPosition(const MazeWalker& walker) {
	Vector2 from = GetCellCenter(walker.fromX, walker.fromY);
	Vector2 to = GetCellCenter(walker.x, walker.y);
	return { from.x + (to.x - from.x) * walker.progress, from.y + (to.y - from.y) * walker.progress };
}

// Next cell
static void MoveWalker(MazeWalker& walker) {
	const int dx[4] = { 0, 1, 0, -1 };
	const int dy[4] = { -1, 0, 1, 0 };
	int options[4];
	int optionCount = 0;

	for (int dir = 0; dir < 4; dir++) {
		bool isBack = walker.x + dx[dir] == walker.fromX && walker.y + dy[dir] == walker.fromY;

		if (!HasWall(maze, walker.x, walker.y, static_cast<Direction>(dir)) && !isBack) {
			options[optionCount++] = dir;
		}
	}

	int nextX = walker.fromX;
	int nextY = walker.fromY;

	// dead end - go back
	if (optionCount > 0) {
		int dir = options[GetRandomValue(0, optionCount - 1)];
		nextX = walker.x + dx[dir];
		nextY = walker.y + dy[dir];
	}

	// look where you walk
	if (nextX != walker.x) {
		walker.isFacingLeft = nextX < walker.x;
	}

	walker.fromX = walker.x;
	walker.fromY = walker.y;
	walker.x = nextX;
	walker.y = nextY;
	walker.progress = 0.0f;
}

// New ember
static void ResetEmber(Ember& ember, float y) {
	ember.position = { RandomFloat(0, static_cast<float>(screenW)), y };
	ember.speed = Scaled(RandomFloat(18, 60));
	ember.size = Scaled(RandomFloat(1.5f, 3.8f));
	ember.phase = RandomFloat(0, 6.28f);
}

// Draw maze once
static void CreateBackground() {
	if (backgroundW != 0) {
		UnloadRenderTexture(background);
	}

	padding = Scaled(36);
	int width = screenW + static_cast<int>(padding * 2);
	int height = screenH + static_cast<int>(padding * 2);

	// at least 10 rows
	float tile = Scaled(36);
	if (tile > height / 21.0f) tile = height / 21.0f;

	int columns = static_cast<int>((width / tile - 1) / 2);
	int rows = static_cast<int>((height / tile - 1) / 2);
	if (columns < MIN_MAZE_SIZE) columns = MIN_MAZE_SIZE;
	if (columns > MAX_MAZE_SIZE) columns = MAX_MAZE_SIZE;
	if (rows > MAX_MAZE_SIZE) rows = MAX_MAZE_SIZE;

	CreateMaze(maze, columns, rows);
	GenerateMaze(maze, static_cast<unsigned int>(time(nullptr)));
	mazeArea = { 0, 0, static_cast<float>(width), static_cast<float>(height) };

	background = LoadRenderTexture(width, height);
	BeginTextureMode(background);
	ClearBackground(COLOR_BACKGROUND);

	// floor behind the maze
	for (float y = 0; y < height; y += tile) {
		for (float x = 0; x < width; x += tile) {
			int variant = static_cast<int>(x / tile + y / tile) % 3;
			DrawTile(GetAssets().tileset, 6 + variant, 0, { x, y, tile, tile });
		}
	}

	DrawMazeWallsIn(maze, mazeArea);
	EndTextureMode();

	backgroundW = screenW;
	backgroundH = screenH;

	for (int i = 0; i < WALKER_COUNT; i++) {
		walkers[i].x = GetRandomValue(0, maze.width - 1);
		walkers[i].y = GetRandomValue(0, maze.height - 1);
		walkers[i].fromX = walkers[i].x;
		walkers[i].fromY = walkers[i].y;
		walkers[i].progress = 1.0f;
		walkers[i].speed = RandomFloat(1.8f, 2.8f);
		walkers[i].isFacingLeft = false;
	}

	for (int i = 0; i < EMBER_COUNT; i++) {
		ResetEmber(embers[i], RandomFloat(0, static_cast<float>(screenH)));
	}

	torch = { screenW / 2.0f, screenH * 0.4f };
}

// Move everything
static void UpdateBackground(float dt) {
	Vector2 mouse = GetMousePosition();

	// torch follows the mouse
	torch.x += (mouse.x - torch.x) * dt * 10;
	torch.y += (mouse.y - torch.y) * dt * 10;

	// maze moves a little against the mouse
	Vector2 target = { (0.5f - mouse.x / screenW) * padding, (0.5f - mouse.y / screenH) * padding };
	offset.x += (target.x - offset.x) * dt * 4;
	offset.y += (target.y - offset.y) * dt * 4;

	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		rippleCenter = mouse;
		rippleAge = 0.0f;
	}

	if (rippleAge < RIPPLE_TIME) {
		rippleAge += dt;
	}

	for (int i = 0; i < WALKER_COUNT; i++) {
		walkers[i].progress += dt * walkers[i].speed;

		if (walkers[i].progress >= 1.0f) {
			MoveWalker(walkers[i]);
		}
	}

	float time = static_cast<float>(GetTime());

	for (int i = 0; i < EMBER_COUNT; i++) {
		embers[i].position.y -= embers[i].speed * dt;
		embers[i].position.x += sinf(time * 1.3f + embers[i].phase) * Scaled(14) * dt;

		if (embers[i].position.y < 0) {
			ResetEmber(embers[i], static_cast<float>(screenH));
		}
	}
}

// Soft light
static void DrawGlow(Vector2 center, float radius, Color color, float strength) {
	Color inner = { color.r, color.g, color.b, static_cast<unsigned char>(255 * strength) };
	DrawCircleGradient(static_cast<int>(center.x), static_cast<int>(center.y), radius, inner, { color.r, color.g, color.b, 0 });
}

// Characters in the maze
static void DrawWalkers() {
	float size = GetCellRectIn(maze, mazeArea, 0, 0).width;

	// walk animation
	int frameRow = static_cast<int>(GetTime() * 4) % 2 == 0 ? 0 : 2;

	for (int i = 0; i < WALKER_COUNT; i++) {
		Vector2 position = GetWalkerPosition(walkers[i]);
		Rectangle tile = { position.x + offset.x - size / 2, position.y + offset.y - size / 2, size, size };

		DrawTile(GetAssets().characters, WALKER_SPRITES[i], frameRow, tile, walkers[i].isFacingLeft);
	}
}

// Lights and sparks
static void DrawLights() {
	float time = static_cast<float>(GetTime());
	float flash = rippleAge < RIPPLE_TIME ? 1.0f - rippleAge / RIPPLE_TIME : 0.0f;
	float torchRadius = Scaled(300) * (1.0f + 0.04f * sinf(time * 3.1f) + 0.3f * flash);

	// colors add up
	BeginBlendMode(BLEND_ADDITIVE);

	DrawGlow(torch, torchRadius, { 255, 150, 60, 255 }, 0.22f + 0.15f * flash);
	DrawGlow(torch, torchRadius * 0.35f, { 255, 200, 120, 255 }, 0.12f);

	// soft light around each character
	for (int i = 0; i < WALKER_COUNT; i++) {
		Vector2 position = GetWalkerPosition(walkers[i]);
		position = { position.x + offset.x, position.y + offset.y };

		DrawGlow(position, Scaled(110), WALKER_COLORS[i], 0.2f);
	}

	for (int i = 0; i < EMBER_COUNT; i++) {
		float flicker = 0.55f + 0.45f * sinf(time * 6.0f + embers[i].phase * 3.0f);
		bool isNearTorch = CheckCollisionPointCircle(embers[i].position, torch, torchRadius);
		float alpha = (isNearTorch ? 0.85f : 0.4f) * flicker;

		DrawCircleV(embers[i].position, embers[i].size, Fade({ 255, 160, 70, 255 }, alpha));
	}

	if (rippleAge < RIPPLE_TIME) {
		float radius = Scaled(30) + Scaled(260) * (1.0f - flash * flash);
		DrawRing(rippleCenter, radius - Scaled(3), radius, 0, 360, 48, Fade(COLOR_ACCENT, 0.5f * flash));
	}

	EndBlendMode();
}

// Dark edges
static void DrawVignette() {
	int edgeY = static_cast<int>(screenH * 0.28f);
	int edgeX = static_cast<int>(screenW * 0.18f);
	Color dark = { 6, 3, 8, 220 };
	Color clear = { 6, 3, 8, 0 };

	DrawRectangleGradientV(0, 0, screenW, edgeY, dark, clear);
	DrawRectangleGradientV(0, screenH - edgeY, screenW, edgeY, clear, dark);
	DrawRectangleGradientH(0, 0, edgeX, screenH, dark, clear);
	DrawRectangleGradientH(screenW - edgeX, 0, edgeX, screenH, clear, dark);
}

// Animated menu background
void DrawMenuBackground() {
	// new maze when the window size changes
	if (backgroundW != screenW || backgroundH != screenH) {
		CreateBackground();
	}

	float dt = GetFrameTime();
	if (dt > 0.05f) dt = 0.05f;
	UpdateBackground(dt);

	// render textures are upside down
	Rectangle source = { 0, 0, static_cast<float>(background.texture.width), -static_cast<float>(background.texture.height) };
	DrawTextureRec(background.texture, source, { offset.x - padding, offset.y - padding }, WHITE);

	DrawRectangle(0, 0, screenW, screenH, { 10, 6, 12, 165 });
	DrawWalkers();
	DrawLights();
	DrawVignette();
}

// Free menu background
void UnloadMenuBackground() {
	if (backgroundW != 0) {
		UnloadRenderTexture(background);
	}

	DestroyMaze(maze);
	backgroundW = 0;
	backgroundH = 0;
}
