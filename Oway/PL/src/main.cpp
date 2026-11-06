#include "raylib.h"
#include "enum.h"
#include "screen.h"
#include "assets.h"
#include "Leaderboard.h"
#include "Network.h"

int main() {
    const int windowWidth = 1280;
    const int windowHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(windowWidth, windowHeight, "Oway");
    SetWindowMinSize(800, 600);
    InitScreens(windowWidth, windowHeight);
    SetTargetFPS(60);
    SetExitKey(0);

    LoadAssets();

    // The database is kept next to the executable. If it can't be opened the game still works without a leaderboard
    InitLeaderboard(TextFormat("%sdata/oway.db", GetApplicationDirectory()));

    // Without the network the game still works, only LAN co-op is unavailable
    InitNetwork();

    ToggleBorderlessWindowed();

    Screen currentScreen = MAINMENU;

    while (!WindowShouldClose() && currentScreen != EXITGAME) {
        if (IsKeyPressed(KEY_F11)) {
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
