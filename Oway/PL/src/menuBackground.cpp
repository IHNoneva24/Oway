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
static const int LIGHT_COUNT = 4;
static const int TRAIL_LENGTH = 16;
static const float RIPPLE_TIME = 0.9f;

// Floating spark
struct Ember {
	Vector2 position;
	float speed;
	float size;
	float phase;
};

// Light walking the maze
struct MazeLight {
	int x;
	int y;
	int fromX;
	int fromY;
	float progress;
	float speed;
	Vector2 trail[TRAIL_LENGTH];
	float trailTimer;
};

static const Color LIGHT_COLORS[LIGHT_COUNT] = {
	{ 255, 190, 90, 255 },
	{ 255, 110, 50, 255 },
	{ 80, 170, 255, 255 },
	{ 200, 150, 255, 255 }
};

static RenderTexture2D background;
static int backgroundW = 0;
static int backgroundH = 0;
static float padding = 0.0f;

static Maze maze;
static Rectangle mazeArea;

static Ember embers[EMBER_COUNT];
static MazeLight lights[LIGHT_COUNT];

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

// Light position
static Vector2 GetLightPosition(const MazeLight& light) {
	Vector2 from = GetCellCenter(light.fromX, light.fromY);
	Vector2 to = GetCellCenter(light.x, light.y);
	return { from.x + (to.x - from.x) * light.progress, from.y + (to.y - from.y) * light.progress };
}

// Next cell
static void MoveLight(MazeLight& light) {
	const int dx[4] = { 0, 1, 0, -1 };
	const int dy[4] = { -1, 0, 1, 0 };
	int options[4];
	int optionCount = 0;

	for (int dir = 0; dir < 4; dir++) {
		bool isBack = light.x + dx[dir] == light.fromX && light.y + dy[dir] == light.fromY;

		if (!HasWall(maze, light.x, light.y, static_cast<Direction>(dir)) && !isBack) {
			options[optionCount++] = dir;
		}
	}

	int nextX = light.fromX;
	int nextY = light.fromY;

	// dead end - go back
	if (optionCount > 0) {
		int dir = options[GetRandomValue(0, optionCount - 1)];
		nextX = light.x + dx[dir];
		nextY = light.y + dy[dir];
	}

	light.fromX = light.x;
	light.fromY = light.y;
	light.x = nextX;
	light.y = nextY;
	light.progress = 0.0f;
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

	for (int i = 0; i < LIGHT_COUNT; i++) {
		lights[i].x = GetRandomValue(0, maze.width - 1);
		lights[i].y = GetRandomValue(0, maze.height - 1);
		lights[i].fromX = lights[i].x;
		lights[i].fromY = lights[i].y;
		lights[i].progress = 1.0f;
		lights[i].speed = RandomFloat(2.2f, 3.6f);
		lights[i].trailTimer = 0.0f;

		for (int t = 0; t < TRAIL_LENGTH; t++) {
			lights[i].trail[t] = GetCellCenter(lights[i].x, lights[i].y);
		}
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

	for (int i = 0; i < LIGHT_COUNT; i++) {
		MazeLight& light = lights[i];
		light.progress += dt * light.speed;

		if (light.progress >= 1.0f) {
			MoveLight(light);
		}

		light.trailTimer -= dt;

		if (light.trailTimer <= 0) {
			light.trailTimer = 0.035f;

			for (int t = TRAIL_LENGTH - 1; t > 0; t--) {
				light.trail[t] = light.trail[t - 1];
			}

			light.trail[0] = GetLightPosition(light);
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

// Lights and sparks
static void DrawLights() {
	float time = static_cast<float>(GetTime());
	float flash = rippleAge < RIPPLE_TIME ? 1.0f - rippleAge / RIPPLE_TIME : 0.0f;
	float torchRadius = Scaled(300) * (1.0f + 0.04f * sinf(time * 3.1f) + 0.3f * flash);

	// colors add up
	BeginBlendMode(BLEND_ADDITIVE);

	DrawGlow(torch, torchRadius, { 255, 150, 60, 255 }, 0.22f + 0.15f * flash);
	DrawGlow(torch, torchRadius * 0.35f, { 255, 200, 120, 255 }, 0.12f);

	for (int i = 0; i < LIGHT_COUNT; i++) {
		for (int t = 1; t < TRAIL_LENGTH; t++) {
			float fade = 1.0f - static_cast<float>(t) / TRAIL_LENGTH;
			Vector2 point = { lights[i].trail[t].x + offset.x, lights[i].trail[t].y + offset.y };
			DrawCircleV(point, Scaled(5) * fade, Fade(LIGHT_COLORS[i], 0.35f * fade));
		}

		Vector2 position = GetLightPosition(lights[i]);
		position = { position.x + offset.x, position.y + offset.y };

		DrawGlow(position, Scaled(110), LIGHT_COLORS[i], 0.2f);
		DrawCircleV(position, Scaled(6), Fade(LIGHT_COLORS[i], 0.9f));
		DrawCircleV(position, Scaled(3), WHITE);
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
	DrawLights();
	DrawVignette();
}
