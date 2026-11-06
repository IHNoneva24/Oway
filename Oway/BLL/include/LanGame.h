#pragma once
#include "Game.h"
#include "Lobby.h"

// Syncing a running race or team game over LAN.
// Every computer moves only its own player and sends where it is; the other player's moves come from the network

enum LanGameEvent {
	LAN_NOTHING,
	LAN_REMATCH,        // the host started a new game (the settings are filled)
	LAN_OTHER_LEFT,     // the other player left the game
	LAN_CONNECTION_LOST // the network stopped working
};

// The other player's state, as it came over the network
struct RemotePlayer {
	bool hasUpdate = false;
	int x = 0;
	int y = 0;
	int moves = 0;
	Direction facing = DIRECTION_RIGHT;
	bool hasEscaped = false;
	float escapeSeconds = 0.0f;
};

// Forgets what was sent and received before. Call when a LAN game starts
void ResetLanGame();

// Sends the local player's state, but only if it changed since the last time
void SendLocalPlayer(const Player& player);

// Tells the other computer (once per game) that the local player reached the exit
void SendLocalEscape(float escapeSeconds);

// Reads the network and updates the other player's state. "settings" is filled on a rematch
LanGameEvent UpdateLanGame(LobbySettings& settings);

// The newest state of the other player
const RemotePlayer& GetRemotePlayer();

// Host: starts a new game for both computers
bool StartLanRematch(const LobbySettings& settings);

// Leaves the game and closes the connection (the other player gets LAN_OTHER_LEFT)
void LeaveLanGame();
