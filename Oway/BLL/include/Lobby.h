#pragma once
#include "Network.h"

// Lobby events
enum LobbyEvent {
	LOBBY_NOTHING,
	LOBBY_PLAYER_JOINED,
	LOBBY_CONNECTED,
	LOBBY_GAME_STARTED,
	LOBBY_WRONG_VERSION,
	LOBBY_PLAYER_LEFT
};

// Game settings
struct LobbySettings {
	int mode = 0;
	int mazeSize = 0;
	unsigned int seed = 0;
};

const int HOST_PLAYER = 0;
const int CLIENT_PLAYER = 1;

// Host lobby
bool HostLobby();
// Join lobby
bool JoinLobby(const char* ipAddress);

// Update lobby
LobbyEvent UpdateLobby(LobbySettings& settings);

// Lobby ready?
bool IsLobbyReady();

// Start game
bool StartLobbyGame(const LobbySettings& settings);

// Leave lobby
void LeaveLobby();

// Local player
int GetLocalPlayer();
