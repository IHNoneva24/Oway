#include "ui.h"

extern int screenW;

bool DrawButton(Rectangle rect, const char* text) {
	Vector2 mouse = GetMousePosition();
	bool isHover = CheckCollisionPointRec(mouse, rect);

	DrawRectangleRounded({ rect.x + 3, rect.y + 3, rect.width, rect.height }, 0.25f, 10, { 0, 0, 0, 120 });
	DrawRectangleRounded(rect, 0.25f, 10, isHover ? COLOR_BUTTON_HOVER : COLOR_BUTTON);

	int fontSize = 24;
	int textWidth = MeasureText(text, fontSize);
	DrawText(text, static_cast<int>(rect.x + rect.width / 2 - textWidth / 2),
		static_cast<int>(rect.y + rect.height / 2 - fontSize / 2), fontSize, RAYWHITE);

	return isHover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
}

void DrawCenteredText(const char* text, int y, int fontSize, Color color) {
	int textWidth = MeasureText(text, fontSize);
	DrawText(text, screenW / 2 - textWidth / 2, y, fontSize, color);
}
