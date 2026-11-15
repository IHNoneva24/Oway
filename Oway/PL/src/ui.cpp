#include "ui.h"
#include "assets.h"
#include "menuBackground.h"
#include <cstring>
#include <cmath>

// from screen.cpp
extern int screenW;
extern int screenH;

static const float BASE_HEIGHT = 1080.0f;
static const float BUTTON_WIDTH = 360.0f;
static const float BUTTON_HEIGHT = 70.0f;

// UI scale
float GetUiScale() {
	float scale = screenH / BASE_HEIGHT;
	return scale < 0.5f ? 0.5f : scale;
}

// Scaled value
float Scaled(float value) {
	return value * GetUiScale();
}

// Draw button
bool DrawButton(Rectangle rect, const char* text) {
	Vector2 mouse = GetMousePosition();
	bool isHover = CheckCollisionPointRec(mouse, rect);

	DrawRectangleRec({ rect.x + Scaled(4), rect.y + Scaled(4), rect.width, rect.height }, { 0, 0, 0, 140 });
	DrawRectangleRec(rect, isHover ? COLOR_BUTTON_HOVER : COLOR_BUTTON);
	DrawRectangleLinesEx(rect, Scaled(3), isHover ? COLOR_ACCENT : Color{ 50, 30, 30, 255 });

	int fontSize = static_cast<int>(Scaled(30));
	int textWidth = MeasureText(text, fontSize);
	DrawText(text, static_cast<int>(rect.x + rect.width / 2 - textWidth / 2),
		static_cast<int>(rect.y + rect.height / 2 - fontSize / 2), fontSize, RAYWHITE);

	return isHover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
}

// Centered button
bool DrawCenteredButton(float y, const char* text) {
	float width = Scaled(BUTTON_WIDTH);
	return DrawButton({ screenW / 2.0f - width / 2, y, width, Scaled(BUTTON_HEIGHT) }, text);
}

// Centered text
void DrawCenteredText(const char* text, float y, float fontSize, Color color) {
	int size = static_cast<int>(fontSize);
	int textWidth = MeasureText(text, size);
	DrawText(text, screenW / 2 - textWidth / 2, static_cast<int>(y), size, color);
}

// Draw background
void DrawDungeonBackground() {
	DrawMenuBackground();
}

// Gold button
bool DrawAccentButton(Rectangle rect, const char* text, bool isEnabled) {
	bool isHover = isEnabled && CheckCollisionPointRec(GetMousePosition(), rect);
	Color background = !isEnabled ? Color{ 70, 56, 60, 255 } : (isHover ? Color{ 255, 210, 110, 255 } : COLOR_ACCENT);

	DrawRectangleRec({ rect.x + Scaled(5), rect.y + Scaled(5), rect.width, rect.height }, { 0, 0, 0, 120 });
	DrawRectangleRec(rect, background);
	DrawRectangleLinesEx(rect, Scaled(3), { 50, 30, 30, 255 });

	int fontSize = static_cast<int>(rect.height * 0.5f);
	int textWidth = MeasureText(text, fontSize);
	DrawText(text, static_cast<int>(rect.x + rect.width / 2 - textWidth / 2),
		static_cast<int>(rect.y + rect.height / 2 - fontSize / 2.0f), fontSize, isEnabled ? Color{ 30, 18, 20, 255 } : COLOR_TEXT_DIM);

	return isHover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
}

// Draw panel
void DrawPanel(Rectangle rect) {
	DrawRectangleRec(rect, COLOR_PANEL);
	DrawRectangleLinesEx(rect, Scaled(3), { 92, 60, 52, 255 });
}

// Draw torch
void DrawTorch(float x, float y, float size) {
	const GameAssets& assets = GetAssets();
	Texture2D frame = assets.torch[GetAnimationFrame()];

	DrawTexturePro(frame, { 0, 0, static_cast<float>(frame.width), static_cast<float>(frame.height) },
		{ x, y, size, size }, { 0, 0 }, 0.0f, WHITE);
}

// Text input
bool UpdateTextInput(char* text, int maxLength, bool digitsOnly) {
	int length = static_cast<int>(strlen(text));
	bool isChanged = false;
	// typed characters
	int key = GetCharPressed();

	while (key > 0) {
		bool isAllowed = digitsOnly ? (key >= '0' && key <= '9') : (key >= 32 && key <= 126);

		if (isAllowed && length < maxLength) {
			text[length++] = static_cast<char>(key);
			text[length] = '\0';
			isChanged = true;
		}

		// typed characters
		key = GetCharPressed();
	}

	// delete character
	if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && length > 0) {
		text[--length] = '\0';
		isChanged = true;
	}

	return isChanged;
}

// Filtered input
bool UpdateFilteredTextInput(char* text, int maxLength, const char* allowedCharacters) {
	int length = static_cast<int>(strlen(text));
	bool isChanged = false;
	// typed characters
	int key = GetCharPressed();

	while (key > 0) {
		// allowed character
		bool isAllowed = key < 128 && strchr(allowedCharacters, key) != nullptr;

		if (isAllowed && length < maxLength) {
			text[length++] = static_cast<char>(key);
			text[length] = 0;
			isChanged = true;
		}

		// typed characters
		key = GetCharPressed();
	}

	// delete character
	if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && length > 0) {
		text[--length] = 0;
		isChanged = true;
	}

	return isChanged;
}

// Draw input box
void DrawTextBox(Rectangle box, const char* text, float fontSize, bool hasError) {
	DrawPanel(box);
	DrawRectangleLinesEx(box, Scaled(3), hasError ? COLOR_ERROR : COLOR_ACCENT);

	// idle animation
	bool isCursorVisible = static_cast<int>(GetTime() * 2) % 2 == 0;
	const char* shown = isCursorVisible ? TextFormat("%s_", text) : text;
	int size = static_cast<int>(fontSize);
	int textWidth = MeasureText(text, size);

	DrawText(shown, static_cast<int>(box.x + box.width / 2 - textWidth / 2),
		static_cast<int>(box.y + box.height / 2 - fontSize / 2), size, RAYWHITE);
}

static const int SPARK_COUNT = 40;
static const float SPARK_LIFE = 0.45f;

// Cursor spark
struct CursorSpark {
	Vector2 position;
	Vector2 speed;
	float life;
};

static CursorSpark sparks[SPARK_COUNT];
static int nextSpark = 0;
static Vector2 lastMouse = { 0, 0 };
static float movedDistance = 0.0f;

// New spark
static void AddSpark(Vector2 position, Vector2 speed) {
	sparks[nextSpark] = { position, speed, SPARK_LIFE };
	nextSpark = (nextSpark + 1) % SPARK_COUNT;
}

// Move and draw sparks
static void DrawSparks(float dt, int pixel) {
	for (int i = 0; i < SPARK_COUNT; i++) {
		if (sparks[i].life <= 0) {
			continue;
		}

		sparks[i].life -= dt;
		sparks[i].position.x += sparks[i].speed.x * dt;
		sparks[i].position.y += sparks[i].speed.y * dt;

		// slow down
		sparks[i].speed.x *= 1.0f - 3.0f * dt;
		sparks[i].speed.y *= 1.0f - 3.0f * dt;

		float fade = sparks[i].life / SPARK_LIFE;
		if (fade < 0) fade = 0;

		// gold first, red at the end
		Color color = { 255, static_cast<unsigned char>(110 + 120 * fade), static_cast<unsigned char>(40 + 80 * fade), 255 };
		int size = fade > 0.5f ? pixel * 2 : pixel;

		DrawRectangle(static_cast<int>(sparks[i].position.x), static_cast<int>(sparks[i].position.y), size, size, Fade(color, fade));
	}
}

// Draw mouse cursor
void DrawCursor() {
	// X - outline, o - fill, the tip is the top left corner
	static const int CURSOR_ROWS = 16;
	static const char* CURSOR_SHAPE[CURSOR_ROWS] = {
		"X",
		" XXX",
		" XooXXX",
		" XoooooXXX",
		"  XoooooooXXX",
		"  XooooooooooXXX",
		"  XoooooooooooX",
		"   XoooooooooX",
		"   XooooooooX",
		"   XoooooooX",
		"    XoooooX",
		"    XooooX",
		"    XoooX",
		"     XoX",
		"     XX",
		"     X"
	};

	float dt = GetFrameTime();
	int pixel = static_cast<int>(Scaled(2) + 0.5f);
	if (pixel < 1) pixel = 1;

	Vector2 mouse = GetMousePosition();
	bool isPressed = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

	// sparks fall off the back while moving
	movedDistance += fabsf(mouse.x - lastMouse.x) + fabsf(mouse.y - lastMouse.y);
	lastMouse = mouse;

	if (movedDistance > Scaled(18)) {
		movedDistance = 0.0f;
		Vector2 tail = { mouse.x + pixel * 7.0f, mouse.y + pixel * 9.0f };
		AddSpark(tail, { Scaled(static_cast<float>(GetRandomValue(-30, 30))), Scaled(static_cast<float>(GetRandomValue(20, 70))) });
	}

	// burst on click
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		const int BURST = 10;

		for (int i = 0; i < BURST; i++) {
			float angle = 6.2832f * i / BURST;
			float speed = Scaled(static_cast<float>(GetRandomValue(160, 280)));
			AddSpark(mouse, { cosf(angle) * speed, sinf(angle) * speed });
		}
	}

	DrawSparks(dt, pixel);

	if (!IsCursorOnScreen()) {
		return;
	}

	int startX = static_cast<int>(mouse.x);
	int startY = static_cast<int>(mouse.y);

	// warm glow under the tip
	float pulse = 0.5f + 0.5f * sinf(static_cast<float>(GetTime()) * 4.0f);
	float glowRadius = pixel * (isPressed ? 16.0f : 11.0f + 2.0f * pulse);
	DrawCircleGradient(startX + pixel * 4, startY + pixel * 5, glowRadius, { 255, 170, 60, 70 }, { 255, 170, 60, 0 });

	Color outline = { 50, 30, 30, 255 };
	Color fill = isPressed ? Color{ 255, 225, 150, 255 } : COLOR_ACCENT;
	Color shine = { 255, 245, 205, 255 };
	Color shadow = { 0, 0, 0, 110 };

	// bright stripe sliding from the tip to the back
	int stripe = static_cast<int>(GetTime() * 26) % 70;

	// shadow first
	for (int row = 0; row < CURSOR_ROWS; row++) {
		for (int column = 0; CURSOR_SHAPE[row][column] != '\0'; column++) {
			if (CURSOR_SHAPE[row][column] != ' ') {
				DrawRectangle(startX + (column + 2) * pixel, startY + (row + 2) * pixel, pixel, pixel, shadow);
			}
		}
	}

	for (int row = 0; row < CURSOR_ROWS; row++) {
		for (int column = 0; CURSOR_SHAPE[row][column] != '\0'; column++) {
			char symbol = CURSOR_SHAPE[row][column];

			if (symbol == ' ') {
				continue;
			}

			Color color = symbol == 'X' ? outline : fill;
			int diagonal = row + column;

			if (symbol == 'o' && (diagonal == stripe || diagonal == stripe + 1)) {
				color = shine;
			}

			DrawRectangle(startX + column * pixel, startY + row * pixel, pixel, pixel, color);
		}
	}
}
