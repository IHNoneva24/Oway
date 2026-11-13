#include "raylib.h"
#include "enum.h"
#include "screen.h"
#include "assets.h"
#include "Leaderboard.h"
#include "Network.h"

// Entry point
int main() {
    const int windowWidth = 1280;
    const int windowHeight = 720;

    // window options
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(windowWidth, windowHeight, "Oway");
    SetWindowMinSize(800, 600);
    InitScreens(windowWidth, windowHeight);
    SetTargetFPS(60);
    SetExitKey(0);

    LoadAssets();

    // exe folder
    InitLeaderboard(TextFormat("%sdata/oway.db", GetApplicationDirectory()));

    InitNetwork();

    // fullscreen
    ToggleBorderlessWindowed();

    Screen currentScreen = MAINMENU;

    while (!WindowShouldClose() && currentScreen != EXITGAME) {
        if (IsKeyPressed(KEY_F11)) {
            // fullscreen
            ToggleBorderlessWindowed();
        }

        BeginDrawing();
        ClearBackground(BLACK);

        UpdateScreen(currentScreen);
        EndDrawing();
    }

    ShutdownNetwork();
    ShutdownLeaderboard();
    UnloadAssets();
    CloseWindow();
}
