#include "screens.h"
#include "ui.h"
#include "assets.h"

// from screen.cpp
extern int screenW;
extern int screenH;
// from screen.cpp
extern GameMode gameMode;
extern bool isLanGame;

// Mode card
struct ModeCard {
	GameMode mode;
	const char* titleLines[2];
	const char* subtitle;
	Color topColor;
	Color bottomColor;
	bool isMultiplayer;
};

static const int CARD_COUNT = 3;

static const ModeCard CARDS[CARD_COUNT] = {
	{ SINGLE_PLAYER, { "SINGLE", "PLAYER" }, "FIND THE WAY OUT ALONE",
		{ 34, 22, 40, 245 }, { 62, 36, 60, 245 }, false },
	{ RACE, { "RACE", "" }, "SAME MAZE. TWO PLAYERS. ONE WINNER.",
		{ 26, 22, 40, 245 }, { 44, 38, 70, 245 }, true },
	{ TEAM_PLAY, { "TEAM", "PLAY" }, "FIRE AND WATER ESCAPE TOGETHER",
		{ 70, 34, 24, 245 }, { 30, 40, 66, 245 }, true }
};

static const Color COLOR_PLAY_HOVER = { 255, 210, 110, 255 };
static const Color COLOR_DISABLED = { 70, 56, 60, 255 };
static const Color COLOR_CARD_BORDER = { 92, 60, 52, 255 };

static int selectedCard = -1;

// Gold button
static bool DrawModeButton(Rectangle rect, const char* text, bool isEnabled) {
	bool isHover = isEnabled && CheckCollisionPointRec(GetMousePosition(), rect);
	Color background = !isEnabled ? COLOR_DISABLED : (isHover ? COLOR_PLAY_HOVER : COLOR_ACCENT);

	DrawRectangleRec({ rect.x + Scaled(5), rect.y + Scaled(5), rect.width, rect.height }, { 0, 0, 0, 120 });
	DrawRectangleRec(rect, background);
	DrawRectangleLinesEx(rect, Scaled(3), { 50, 30, 30, 255 });

	int fontSize = static_cast<int>(rect.height * 0.5f);
	int textWidth = MeasureText(text, fontSize);
	DrawText(text, static_cast<int>(rect.x + rect.width / 2 - textWidth / 2),
		static_cast<int>(rect.y + rect.height / 2 - fontSize / 2.0f), fontSize, isEnabled ? Color{ 30, 18, 20, 255 } : COLOR_TEXT_DIM);

	return isHover && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
}

// Draw character
static void DrawCharacter(int column, float centerX, float bottom, float size, bool flipX) {
	// idle animation
	int frameRow = static_cast<int>(GetTime() * 2) % 2 == 0 ? 0 : 2;
	DrawTile(GetAssets().characters, column, frameRow, { centerX - size / 2, bottom - size, size, size }, flipX);
}

// Stretched texture
static void DrawTexture16(Texture2D texture, Rectangle destination) {
	DrawTexturePro(texture, { 0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height) },
		destination, { 0, 0 }, 0.0f, WHITE);
}

// Card picture
static void DrawCardArt(int index, Rectangle card) {
	const GameAssets& assets = GetAssets();
	float centerX = card.x + card.width / 2;
	float floorY = card.y + card.height * 0.62f;

	Color ray = { 240, 180, 60, 22 };
	DrawTriangle({ centerX, card.y }, { centerX - card.width * 0.45f, floorY }, { centerX - card.width * 0.15f, floorY }, ray);
	DrawTriangle({ centerX, card.y }, { centerX + card.width * 0.15f, floorY }, { centerX + card.width * 0.45f, floorY }, ray);

	float big = card.width * 0.62f;
	float small = card.width * 0.44f;

	switch (index) {
	case 0:
		DrawTorch(card.x + card.width * 0.06f, floorY - big * 0.85f, card.width * 0.2f);
		DrawTorch(card.x + card.width * 0.74f, floorY - big * 0.85f, card.width * 0.2f);
		DrawCharacter(0, centerX, floorY, big, false);
		break;

	case 1:
		DrawTile(assets.tileset, 9, 3, { centerX - small * 0.2f, floorY - small * 0.4f, small * 0.4f, small * 0.4f });
		DrawCharacter(0, card.x + card.width * 0.24f, floorY, small * 0.9f, false);
		DrawCharacter(3, card.x + card.width * 0.76f, floorY, small * 0.9f, true);
		break;

	case 2:
		DrawTexture16(assets.flame[GetAnimationFrame() % 3],
			{ card.x + card.width * 0.08f, floorY - small * 0.9f, small * 0.3f, small * 0.6f });
		DrawTile(assets.tileset, 7, 8, { card.x + card.width * 0.74f, floorY - small * 0.55f, small * 0.4f, small * 0.4f });
		DrawCharacter(1, card.x + card.width * 0.33f, floorY, small, false);
		DrawCharacter(3, card.x + card.width * 0.67f, floorY, small, true);
		break;
	}
}

// Card title
static void DrawCardTitle(const ModeCard& mode, Rectangle card) {
	int titleSize = static_cast<int>(card.width * 0.19f);
	int subtitleSize = static_cast<int>(card.width * 0.045f);
	int lineCount = mode.titleLines[1][0] == '\0' ? 1 : 2;
	float y = card.y + card.height * 0.66f + (2 - lineCount) * titleSize * 0.5f;

	for (int line = 0; line < lineCount; line++) {
		const char* text = mode.titleLines[line];
		int x = static_cast<int>(card.x + card.width / 2 - MeasureText(text, titleSize) / 2);

		DrawText(text, x + 4, static_cast<int>(y) + 4, titleSize, BLACK);
		DrawText(text, x, static_cast<int>(y), titleSize, COLOR_ACCENT);
		y += titleSize * 0.95f;
	}

	DrawText(mode.subtitle, static_cast<int>(card.x + card.width / 2 - MeasureText(mode.subtitle, subtitleSize) / 2),
		static_cast<int>(y + Scaled(8)), subtitleSize, COLOR_TEXT_DIM);
}

// Centered text
static void DrawCenteredTextIn(const char* text, Rectangle area, float y, float fontSize) {
	int size = static_cast<int>(fontSize);
	DrawText(text, static_cast<int>(area.x + area.width / 2 - MeasureText(text, size) / 2), static_cast<int>(y), size, RAYWHITE);
}

// Play choice
enum MultiplayerChoice {
	CHOICE_NONE,
	CHOICE_LOCAL,
	CHOICE_LAN
};

// Local or LAN
static MultiplayerChoice DrawMultiplayerChoice(Rectangle card) {
	float width = card.width * 0.7f;
	float height = card.width * 0.17f;
	float x = card.x + card.width / 2 - width / 2;
	float y = card.y + card.height * 0.32f;

	DrawCenteredTextIn("HOW DO YOU PLAY?", card, y - height * 0.9f, card.width * 0.07f);

	MultiplayerChoice choice = CHOICE_NONE;

	if (DrawModeButton({ x, y, width, height }, "LOCAL", true)) {
		choice = CHOICE_LOCAL;
	}

	if (DrawModeButton({ x, y + height * 1.7f, width, height }, "CO-OP", true)) {
		choice = CHOICE_LAN;
	}

	DrawCenteredTextIn("Same keyboard", card, y + height + Scaled(4), card.width * 0.045f);
	DrawCenteredTextIn("Two computers, same network", card, y + height * 2.7f + Scaled(4), card.width * 0.045f);

	return choice;
}

// Mode select
void DrawModeSelect(Screen& currentScreen) {
	DrawDungeonBackground();

	float cardWidth = screenW * 0.25f;
	if (cardWidth > screenH * 0.42f) cardWidth = screenH * 0.42f;
	float cardHeight = cardWidth * 1.6f;
	float gap = cardWidth * 0.1f;
	float startX = screenW / 2.0f - (cardWidth * CARD_COUNT + gap * (CARD_COUNT - 1)) / 2;
	float cardY = screenH / 2.0f - cardHeight / 2 - Scaled(20);

	Vector2 mouse = GetMousePosition();
	bool isOverCard = false;

	for (int i = 0; i < CARD_COUNT; i++) {
		Rectangle card = { startX + i * (cardWidth + gap), cardY, cardWidth, cardHeight };
		bool isHover = CheckCollisionPointRec(mouse, card);
		bool isSelected = selectedCard == i;
		isOverCard = isOverCard || isHover;

		if (isHover && !isSelected) {
			card.y -= Scaled(10);
		}

		DrawRectangleRec({ card.x + Scaled(8), card.y + Scaled(8), card.width, card.height }, { 0, 0, 0, 70 });

		if (i == 2) {
			DrawRectangleGradientH(static_cast<int>(card.x), static_cast<int>(card.y), static_cast<int>(card.width),
				static_cast<int>(card.height), CARDS[i].topColor, CARDS[i].bottomColor);
		}
		else {
			DrawRectangleGradientV(static_cast<int>(card.x), static_cast<int>(card.y), static_cast<int>(card.width),
				static_cast<int>(card.height), CARDS[i].topColor, CARDS[i].bottomColor);
		}

		DrawRectangleLinesEx(card, Scaled(3), COLOR_CARD_BORDER);

		// clip to card
		BeginScissorMode(static_cast<int>(card.x), static_cast<int>(card.y), static_cast<int>(card.width), static_cast<int>(card.height));
		DrawCardArt(i, card);
		EndScissorMode();

		DrawCardTitle(CARDS[i], card);

		if (isSelected) {
			DrawRectangleRec(card, { 0, 0, 0, 170 });
			DrawRectangleLinesEx(card, Scaled(5), COLOR_ACCENT);

			MultiplayerChoice choice = DrawMultiplayerChoice(card);

			if (choice != CHOICE_NONE) {
				gameMode = CARDS[i].mode;
				isLanGame = choice == CHOICE_LAN;
				selectedCard = -1;
				currentScreen = isLanGame ? LOBBY : SIZESELECT;
			}
		}
		else if (isHover) {
			DrawRectangleLinesEx(card, Scaled(4), COLOR_ACCENT);

			if (!CARDS[i].isMultiplayer) {
				float buttonWidth = card.width * 0.45f;
				DrawModeButton({ card.x + card.width / 2 - buttonWidth / 2, card.y + card.height - card.width * 0.22f,
					buttonWidth, card.width * 0.14f }, "PLAY", true);
			}
		}

		if (isHover && !isSelected && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
			if (CARDS[i].isMultiplayer) {
				selectedCard = i;
			}
			else {
				gameMode = CARDS[i].mode;
				isLanGame = false;
				selectedCard = -1;
				currentScreen = SIZESELECT;
			}
		}
	}

	if (!isOverCard && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
		selectedCard = -1;
	}

	DrawCenteredText("SELECT A GAME MODE", cardY + cardHeight + Scaled(40), Scaled(34), COLOR_ACCENT);

	if (DrawButton({ Scaled(30), screenH - Scaled(100), Scaled(200), Scaled(70) }, "Back") || IsKeyPressed(KEY_ESCAPE)) {
		if (selectedCard != -1) {
			selectedCard = -1;
		}
		else {
			currentScreen = MAINMENU;
		}
	}
}
