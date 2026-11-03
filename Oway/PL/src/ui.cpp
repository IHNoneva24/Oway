#include "ui.h"
#include "assets.h"

extern int screenW;
extern int screenH;

static const float BASE_HEIGHT = 1080.0f;
static const float BUTTON_WIDTH = 360.0f;
static const float BUTTON_HEIGHT = 70.0f;

float GetUiScale() {
	float scale = screenH / BASE_HEIGHT;
	return scale < 0.5f ? 0.5f : scale;
}

float Scaled(float value) {
	return value * GetUiScale();
}

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

bool DrawCenteredButton(float y, const char* text) {
	float width = Scaled(BUTTON_WIDTH);
	return DrawButton({ screenW / 2.0f - width / 2, y, width, Scaled(BUTTON_HEIGHT) }, text);
}

void DrawCenteredText(const char* text, float y, float fontSize, Color color) {
	int size = static_cast<int>(fontSize);
	int textWidth = MeasureText(text, size);
	DrawText(text, screenW / 2 - textWidth / 2, static_cast<int>(y), size, color);
}

void DrawDungeonBackground() {
	const GameAssets& assets = GetAssets();
	float tile = Scaled(64);

	for (float y = 0; y < screenH; y += tile) {
		for (float x = 0; x < screenW; x += tile) {
			int variant = static_cast<int>(x / tile + y / tile) % 3;
			DrawTile(assets.tileset, 6 + variant, 0, { x, y, tile, tile });
		}
	}

	DrawRectangle(0, 0, screenW, screenH, { 10, 6, 12, 150 });
}

void DrawPanel(Rectangle rect) {
	DrawRectangleRec(rect, COLOR_PANEL);
	DrawRectangleLinesEx(rect, Scaled(3), { 92, 60, 52, 255 });
}

void DrawTorch(float x, float y, float size) {
	const GameAssets& assets = GetAssets();
	Texture2D frame = assets.torch[GetAnimationFrame()];

	DrawTexturePro(frame, { 0, 0, static_cast<float>(frame.width), static_cast<float>(frame.height) },
		{ x, y, size, size }, { 0, 0 }, 0.0f, WHITE);
}
