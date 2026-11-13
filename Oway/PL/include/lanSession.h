#pragma once
#include "enum.h"
#include "Lobby.h"

// Start LAN game
bool StartLanSession(const LobbySettings& settings, Screen& currentScreen);

// Host rematch
bool HostLanRematch(int mazeSize, Screen& currentScreen);

// LAN events
void UpdateLanSession(Screen& currentScreen);

// Leave menu
bool DrawLanLeaveMenu(Screen& currentScreen);

// Toggle menu
void ToggleLanLeaveMenu();

// Player left
bool DrawLanNotice(Screen& currentScreen);
