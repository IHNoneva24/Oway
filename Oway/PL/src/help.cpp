#include "screens.h"
#include "ui.h"

extern int screenW;
extern int screenH;

static const char* HELP_LINES[] = {
	"You start in the top left corner of the dungeon.",
	"Find the ladder in the bottom right corner to escape.",
	"You can't walk through walls.",
	"Choose a maze size from 10x10 to 50x50.",
	"Every game generates a brand new maze.",
	"",
	"TEAM PLAY  -  Fire: WASD,  Water: Arrow keys",
	"Lava is safe only for Fire, water only for Water.",
	"Wrong pools and raised spikes send you back to the start.",
	"Stand on a plate to open the gates for your friend.",
	"Collect all gems, then escape on the ladder together.",
	"",
	"WASD & Arrow keys  -  move",
	"ESC  -  pause",
	"F11  -  fullscreen / window"
};

static const int HELP_LINE_COUNT = sizeof(HELP_LINES) / sizeof(HELP_LINES[0]);

void DrawHelp(Screen& currentScreen) {
	DrawDungeonBackground();

	float panelWidth = Scaled(1000);
	float panelHeight = Scaled(980);
	DrawPanel({ screenW / 2.0f - panelWidth / 2, screenH / 2.0f - panelHeight / 2, panelWidth, panelHeight });

	float y = screenH / 2.0f - panelHeight / 2 + Scaled(40);
	DrawCenteredText("HOW TO PLAY", y, Scaled(60), COLOR_ACCENT);

	y += Scaled(110);

	for (int i = 0; i < HELP_LINE_COUNT; i++) {
		DrawCenteredText(HELP_LINES[i], y, Scaled(30), RAYWHITE);
		y += Scaled(44);
	}

	if (DrawCenteredButton(screenH / 2.0f + panelHeight / 2 - Scaled(100), "Back") || IsKeyPressed(KEY_ESCAPE)) {
		currentScreen = MAINMENU;
	}
}
