#include "Lobby.h"
#include "Maze.h"

static bool isHelloSent = false;
static bool isReady = false;

// Reset lobby
static void ResetLobby() {
	isHelloSent = false;
	isReady = false;
}

// Send hello
static void SendHello() {
	NetworkMessage hello;
	hello.type = MESSAGE_HELLO;
	hello.value = PROTOCOL_VERSION;

	SendNetworkMessage(hello);
	isHelloSent = true;
}

// Host lobby
bool HostLobby() {
	ResetLobby();
	return StartHosting(DEFAULT_PORT);
}

// Join lobby
bool JoinLobby(const char* ipAddress) {
	ResetLobby();
	return StartJoining(ipAddress, DEFAULT_PORT);
}

// Update lobby
LobbyEvent UpdateLobby(LobbySettings& settings) {
	NetworkState stateBefore = GetNetworkState();
	UpdateNetwork();

	if (GetNetworkState() == NETWORK_ERROR) {
		bool wasConnected = stateBefore == NETWORK_CONNECTED;
		isReady = false;
		return wasConnected ? LOBBY_PLAYER_LEFT : LOBBY_NOTHING;
	}

	if (GetNetworkState() != NETWORK_CONNECTED) {
		return LOBBY_NOTHING;
	}

	if (!isHelloSent) {
		SendHello();
	}

	NetworkMessage message;

	while (ReceiveNetworkMessage(message)) {
		switch (message.type) {
		case MESSAGE_HELLO:
			if (message.value != PROTOCOL_VERSION) {
				Disconnect();
				return LOBBY_WRONG_VERSION;
			}

			isReady = true;
			return IsHost() ? LOBBY_PLAYER_JOINED : LOBBY_CONNECTED;

		case MESSAGE_START:
			if (!IsHost() && isReady && IsValidMazeSize(message.value)) {
				settings.mode = message.player;
				settings.mazeSize = message.value;
				settings.seed = message.seed;
				return LOBBY_GAME_STARTED;
			}
			break;

		case MESSAGE_BYE:
			Disconnect();
			return LOBBY_PLAYER_LEFT;

		default:
			break;
		}
	}

	return LOBBY_NOTHING;
}

// Lobby ready?
bool IsLobbyReady() {
	return isReady && GetNetworkState() == NETWORK_CONNECTED;
}

// Start game
bool StartLobbyGame(const LobbySettings& settings) {
	if (!IsHost() || !IsLobbyReady() || !IsValidMazeSize(settings.mazeSize)) {
		return false;
	}

	NetworkMessage start;
	start.type = MESSAGE_START;
	start.player = static_cast<unsigned char>(settings.mode);
	start.value = settings.mazeSize;
	start.seed = settings.seed;

	return SendNetworkMessage(start);
}

// Leave lobby
void LeaveLobby() {
	Disconnect();
	ResetLobby();
}

// Local player
int GetLocalPlayer() {
	return IsHost() ? HOST_PLAYER : CLIENT_PLAYER;
}
