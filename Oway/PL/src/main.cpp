#include "raylib.h"
#include "enum.h"
#include "screen.h"

int main() {
    const int windowWidth = 900;
    const int windowHeight = 900;

    InitWindow(windowWidth, windowHeight, "Oway");
    InitScreens(windowWidth, windowHeight);
    SetTargetFPS(60);
    SetExitKey(0);

    Screen currentScreen = MAINMENU;

    while (!WindowShouldClose() && currentScreen != EXITGAME) {
        BeginDrawing();
        ClearBackground(BLACK);

        UpdateScreen(currentScreen);
        EndDrawing();
    }

    CloseWindow();
}
