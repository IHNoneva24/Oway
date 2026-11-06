#pragma once
#include "enum.h"
#include "Lobby.h"

// Shared by the race and team screens when the game is played over LAN

// Starts the race or team game with the settings both computers share and opens its screen
bool StartLanSession(const LobbySettings& settings, Screen& currentScreen);

// Host: starts a new maze of the same mode and size on both computers
bool HostLanRematch(int mazeSize, Screen& currentScreen);

// Reacts to a rematch or the other player leaving. Call once per frame on the LAN game and win screens
void UpdateLanSession(Screen& currentScreen);

// ESC menu of a LAN game. The game is not paused, because the other player keeps playing.
// Returns true while the menu is open
bool DrawLanLeaveMenu(Screen& currentScreen);

// Opens or closes the ESC menu
void ToggleLanLeaveMenu();

// "The other player left" message with a button back to the menu. Returns true while it is shown
bool DrawLanNotice(Screen& currentScreen);
