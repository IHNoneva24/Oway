#include "raylib.h"
#include "enum.h"
#include "screens.h"

int main() {
    InitWindow(800, 600, "Oway");
    SetExitKey(NULL);

    Screen currentScreen = MAINMENU;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        EndDrawing();
    }

    CloseWindow();
}
