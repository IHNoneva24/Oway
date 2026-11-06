#pragma once
#include "Network.h"

// LAN lobby logic - the host waits for a player, checks the version and starts the game for both

enum LobbyEvent {
	LOBBY_NOTHING,
	LOBBY_PLAYER_JOINED,  // host: a player connected and has the same game version
	LOBBY_CONNECTED,      // client: connected to the host, waiting for the start
	LOBBY_GAME_STARTED,   // client: the host started the game (the settings are filled)
	LOBBY_WRONG_VERSION,  // the other player has a different version of the game
	LOBBY_PLAYER_LEFT     // the connection was closed or lost
};

// What the host sends to start a game
struct LobbySettings {
	int mode = 0;          // game mode, as a number (the PL decides what the number means)
	int mazeSize = 0;
	unsigned int seed = 0; // same seed = same maze on both computers
};

// The host is always player 1, the player who joins is player 2
const int HOST_PLAYER = 0;
const int CLIENT_PLAYER = 1;

// Starts hosting or joining and resets the lobby
bool HostLobby();
bool JoinLobby(const char* ipAddress);

// Handles the network and the lobby messages. Call once per frame while in the lobby
LobbyEvent UpdateLobby(LobbySettings& settings);

// True when both players are connected and said hello
bool IsLobbyReady();

// Host: sends the settings to the other player. Returns false if nobody is connected or the size is wrong
bool StartLobbyGame(const LobbySettings& settings);

// Leaves the lobby and closes the connection
void LeaveLobby();

// The number of this computer's player (HOST_PLAYER or CLIENT_PLAYER)
int GetLocalPlayer();
