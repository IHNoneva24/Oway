#pragma once
#include "Game.h"
#include "Lobby.h"

// LAN events
enum LanGameEvent {
	LAN_NOTHING,
	LAN_REMATCH,
	LAN_OTHER_LEFT,
	LAN_CONNECTION_LOST
};

// Other player
struct RemotePlayer {
	bool hasUpdate = false;
	int x = 0;
	int y = 0;
	int moves = 0;
	Direction facing = DIRECTION_RIGHT;
	bool hasEscaped = false;
	float escapeSeconds = 0.0f;
};

// Reset sync
void ResetLanGame();

// Send position
void SendLocalPlayer(const Player& player);

// Send escape
void SendLocalEscape(float escapeSeconds);

// Update sync
LanGameEvent UpdateLanGame(LobbySettings& settings);

// Other player
const RemotePlayer& GetRemotePlayer();

// Start rematch
bool StartLanRematch(const LobbySettings& settings);

// Leave game
void LeaveLanGame();
