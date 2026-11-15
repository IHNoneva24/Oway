#include "screens.h"
#include "ui.h"
#include "mazeView.h"
#include "Game.h"
#include "TeamGame.h"
#include "RaceGame.h"
#include <ctime>

// from screen.cpp
extern int screenW;
extern int screenH;
// from screen.cpp
extern GameMode gameMode;

static const int MAX_INPUT_LENGTH = 2;
static const int PRESET_COUNT = 5;

static const int PRESET_SIZES[PRESET_COUNT] = { 10, 15, 25, 35, 50 };
static const char* PRESET_NAMES[PRESET_COUNT] = { "QUICK", "SMALL", "MEDIUM", "LARGE", "EPIC" };

static char sizeInput[MAX_INPUT_LENGTH + 1] = "20";
static int mazeSize = 20;
static bool showError = false;
static bool isDragging = false;

static Maze previewMaze;
static RenderTexture2D preview;
static int previewSize = 0;
static int previewPixels = 0;

// Set size
static void SetSize(int size) {
	if (size < MIN_MAZE_SIZE) size = MIN_MAZE_SIZE;
	if (size > MAX_MAZE_SIZE) size = MAX_MAZE_SIZE;

	TextCopy(sizeInput, TextFormat("%d", size));
	mazeSize = size;
	showError = false;
}

// Start game
static bool StartSelectedGame(int size) {
	unsigned int seed = static_cast<unsigned int>(time(nullptr));

	if (gameMode == TEAM_PLAY) {
		return StartTeamSession(GetCurrentTeamSession(), size, seed);
	}

	if (gameMode == RACE) {
		return StartRaceSession(GetCurrentRaceSession(), size, seed);
	}

	return StartGameSession(GetCurrentSession(), size, seed);
}

// Screen title
static const char* GetSizeSelectTitle() {
	switch (gameMode) {
	case RACE:      return "Race - choose maze size";
	case TEAM_PLAY: return "Team play - choose maze size";
	default:        return "Choose maze size";
	}
}

// Game screen
static Screen GetGameScreen() {
	switch (gameMode) {
	case RACE:      return RACEGAME;
	case TEAM_PLAY: return TEAMGAME;
	default:        return GAME;
	}
}

// Difficulty name
static const char* GetDifficultyName(int size) {
	if (size <= 15) return "EASY";
	if (size <= 25) return "NORMAL";
	if (size <= 38) return "HARD";
	return "BRUTAL";
}

// Difficulty color
static Color GetDifficultyColor(int size) {
	if (size <= 15) return { 120, 210, 120, 255 };
	if (size <= 25) return COLOR_ACCENT;
	if (size <= 38) return { 245, 130, 60, 255 };
	return COLOR_ERROR;
}

// Centered text
static void DrawTextIn(const char* text, Rectangle area, float y, float fontSize, Color color) {
	int size = static_cast<int>(fontSize);
	DrawText(text, static_cast<int>(area.x + area.width / 2 - MeasureText(text, size) / 2), static_cast<int>(y), size, color);
}

// Maze preview
static void DrawPreview(Rectangle panel) {
	DrawRectangleRec({ panel.x + Scaled(8), panel.y + Scaled(8), panel.width, panel.height }, { 0, 0, 0, 110 });
	DrawPanel(panel);

	float inset = Scaled(18);
	int pixels = static_cast<int>(panel.width - inset * 2);

	// draw the maze again only when the size changes
	if (previewSize != mazeSize || previewPixels != pixels) {
		if (previewPixels != 0) {
			UnloadRenderTexture(preview);
		}

		preview = LoadRenderTexture(pixels, pixels);
		CreateMaze(previewMaze, mazeSize, mazeSize);
		GenerateMaze(previewMaze, static_cast<unsigned int>(mazeSize));

		Rectangle area = { 0, 0, static_cast<float>(pixels), static_cast<float>(pixels) };

		BeginTextureMode(preview);
		ClearBackground(BLANK);
		DrawMazeWallsIn(previewMaze, area);
		DrawExitIn(previewMaze, area, true);
		DrawRectangleRec(GetCellRectIn(previewMaze, area, 0, 0), { 120, 210, 255, 220 });
		EndTextureMode();

		previewSize = mazeSize;
		previewPixels = pixels;
	}

	// render textures are upside down
	Rectangle source = { 0, 0, static_cast<float>(pixels), -static_cast<float>(pixels) };
	DrawTextureRec(preview.texture, source, { panel.x + inset, panel.y + inset }, WHITE);

	DrawTextIn("PREVIEW", panel, panel.y - Scaled(42), Scaled(26), COLOR_ACCENT);
	DrawTextIn("Every game gets a brand new maze", panel, panel.y + panel.height + Scaled(16), Scaled(20), COLOR_TEXT_DIM);
}

// Size slider
static void DrawSlider(Rectangle track) {
	Vector2 mouse = GetMousePosition();
	float knobRadius = Scaled(20);
	Rectangle hitArea = { track.x - knobRadius, track.y - knobRadius * 1.5f, track.width + knobRadius * 2, knobRadius * 3 };
	bool isHover = CheckCollisionPointRec(mouse, hitArea);

	if (isHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		isDragging = true;
	}

	if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
		isDragging = false;
	}

	if (isDragging) {
		float part = (mouse.x - track.x) / track.width;
		SetSize(MIN_MAZE_SIZE + static_cast<int>(part * (MAX_MAZE_SIZE - MIN_MAZE_SIZE) + 0.5f));
	}

	float knobX = track.x + track.width * (mazeSize - MIN_MAZE_SIZE) / (MAX_MAZE_SIZE - MIN_MAZE_SIZE);
	float knobY = track.y + track.height / 2;

	DrawRectangleRounded(track, 1.0f, 8, { 12, 8, 14, 230 });
	DrawRectangleRounded({ track.x, track.y, knobX - track.x, track.height }, 1.0f, 8, GetDifficultyColor(mazeSize));

	// marks 10 20 30 40 50
	for (int value = MIN_MAZE_SIZE; value <= MAX_MAZE_SIZE; value += 10) {
		float x = track.x + track.width * (value - MIN_MAZE_SIZE) / (MAX_MAZE_SIZE - MIN_MAZE_SIZE);
		Rectangle label = { x - Scaled(30), 0, Scaled(60), 0 };

		DrawRectangleRec({ x - Scaled(1), track.y + track.height + Scaled(6), Scaled(2), Scaled(8) }, COLOR_TEXT_DIM);
		DrawTextIn(TextFormat("%d", value), label, track.y + track.height + Scaled(18), Scaled(20), COLOR_TEXT_DIM);
	}

	float radius = isHover || isDragging ? knobRadius * 1.15f : knobRadius;
	DrawCircleV({ knobX + Scaled(3), knobY + Scaled(3) }, radius, { 0, 0, 0, 120 });
	DrawCircleV({ knobX, knobY }, radius, COLOR_ACCENT);
	DrawCircleLines(static_cast<int>(knobX), static_cast<int>(knobY), radius, { 50, 30, 30, 255 });
}

// Preset buttons
static void DrawPresets(Rectangle row) {
	float gap = Scaled(12);
	float width = (row.width - gap * (PRESET_COUNT - 1)) / PRESET_COUNT;

	for (int i = 0; i < PRESET_COUNT; i++) {
		Rectangle button = { row.x + i * (width + gap), row.y, width, row.height };
		bool isHover = CheckCollisionPointRec(GetMousePosition(), button);
		bool isActive = mazeSize == PRESET_SIZES[i];
		Color background = isActive ? COLOR_ACCENT : (isHover ? COLOR_BUTTON_HOVER : COLOR_BUTTON);
		Color textColor = isActive ? Color{ 30, 18, 20, 255 } : RAYWHITE;

		if (isHover && !isActive) {
			button.y -= Scaled(4);
		}

		DrawRectangleRec({ button.x + Scaled(4), button.y + Scaled(4), button.width, button.height }, { 0, 0, 0, 120 });
		DrawRectangleRec(button, background);
		DrawRectangleLinesEx(button, Scaled(3), isHover || isActive ? COLOR_ACCENT : Color{ 50, 30, 30, 255 });

		DrawTextIn(TextFormat("%d", PRESET_SIZES[i]), button, button.y + button.height * 0.14f, button.height * 0.42f, textColor);
		DrawTextIn(PRESET_NAMES[i], button, button.y + button.height * 0.62f, button.height * 0.22f, isActive ? textColor : COLOR_TEXT_DIM);

		if (isHover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
			SetSize(PRESET_SIZES[i]);
		}
	}
}

// Size select
void DrawSizeSelect(Screen& currentScreen) {
	DrawDungeonBackground();

	// typed number
	if (UpdateTextInput(sizeInput, MAX_INPUT_LENGTH, true)) {
		showError = false;
		TryParseMazeSize(sizeInput, mazeSize);
	}

	if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN) || IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)
		|| GetMouseWheelMove() < 0) {
		SetSize(mazeSize - 1);
	}

	if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP) || IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)
		|| GetMouseWheelMove() > 0) {
		SetSize(mazeSize + 1);
	}

	float titleY = screenH * 0.09f;
	DrawCenteredText(GetSizeSelectTitle(), titleY + Scaled(5), Scaled(70), BLACK);
	DrawCenteredText(GetSizeSelectTitle(), titleY, Scaled(70), COLOR_ACCENT);
	DrawCenteredText("Drag, scroll, type or use the arrow keys", titleY + Scaled(88), Scaled(26), COLOR_TEXT_DIM);

	// controls on the left, preview on the right
	float columnWidth = Scaled(660);
	float previewSide = Scaled(540);
	float gap = Scaled(90);
	float left = screenW / 2.0f - (columnWidth + gap + previewSide) / 2;
	float top = screenH * 0.29f;
	Rectangle column = { left, top, columnWidth, 0 };

	float buttonWidth = Scaled(110);
	float boxHeight = Scaled(120);

	if (DrawButton({ left, top, buttonWidth, boxHeight }, "-")) {
		SetSize(mazeSize - 1);
	}

	if (DrawButton({ left + columnWidth - buttonWidth, top, buttonWidth, boxHeight }, "+")) {
		SetSize(mazeSize + 1);
	}

	int size = 0;
	bool isValid = TryParseMazeSize(sizeInput, size);
	Rectangle box = { left + columnWidth / 2 - Scaled(150), top, Scaled(300), boxHeight };

	DrawPanel(box);
	DrawRectangleLinesEx(box, Scaled(3), showError ? COLOR_ERROR : COLOR_ACCENT);
	DrawTextIn(isValid ? TextFormat("%d x %d", size, size) : TextFormat("%s_", sizeInput), box,
		top + boxHeight / 2 - Scaled(32), Scaled(64), isValid ? RAYWHITE : COLOR_TEXT_DIM);

	float infoY = top + boxHeight + Scaled(22);

	if (showError) {
		DrawTextIn(TextFormat("Please enter a size between %d and %d", MIN_MAZE_SIZE, MAX_MAZE_SIZE), column, infoY, Scaled(26), COLOR_ERROR);
	}
	else {
		// difficulty in color, rooms in gray
		const char* difficulty = GetDifficultyName(mazeSize);
		const char* rooms = TextFormat("  -  %d rooms", mazeSize * mazeSize);
		int fontSize = static_cast<int>(Scaled(28));
		int difficultyWidth = MeasureText(difficulty, fontSize);
		int x = static_cast<int>(left + columnWidth / 2 - (difficultyWidth + MeasureText(rooms, fontSize)) / 2);

		DrawText(difficulty, x, static_cast<int>(infoY), fontSize, GetDifficultyColor(mazeSize));
		DrawText(rooms, x + difficultyWidth, static_cast<int>(infoY), fontSize, COLOR_TEXT_DIM);
	}

	DrawSlider({ left + Scaled(24), top + Scaled(225), columnWidth - Scaled(48), Scaled(14) });
	DrawPresets({ left, top + Scaled(320), columnWidth, Scaled(92) });
	DrawPreview({ left + columnWidth + gap, top, previewSide, previewSide });

	if (DrawAccentButton({ left, top + Scaled(460), columnWidth, Scaled(96) }, "START") || IsKeyPressed(KEY_ENTER)) {
		if (isValid && StartSelectedGame(size)) {
			currentScreen = GetGameScreen();
		}
		else {
			showError = true;
		}
	}

	if (DrawButton({ Scaled(30), screenH - Scaled(100), Scaled(200), Scaled(70) }, "Back") || IsKeyPressed(KEY_ESCAPE)) {
		SetSize(mazeSize);
		currentScreen = MODESELECT;
	}
}
