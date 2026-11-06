#include "LanGame.h"

// The last state that was sent, so nothing is sent while the player stands still
static Player lastSent;
static bool hasSentState = false;
static bool isEscapeSent = false;
static RemotePlayer remote;

void ResetLanGame() {
	lastSent = Player{};
	hasSentState = false;
	isEscapeSent = false;
	remote = RemotePlayer{};
}

const RemotePlayer& GetRemotePlayer() {
	return remote;
}

void SendLocalPlayer(const Player& player) {
	bool isChanged = !hasSentState || player.x != lastSent.x || player.y != lastSent.y
		|| player.moves != lastSent.moves || player.facing != lastSent.facing;

	if (!isChanged) {
		return;
	}

	NetworkMessage move;
	move.type = MESSAGE_MOVE;
	move.player = static_cast<unsigned char>(GetLocalPlayer());
	move.x = static_cast<short>(player.x);
	move.y = static_cast<short>(player.y);
	move.value = player.moves;
	move.seed = static_cast<unsigned int>(player.facing);

	if (SendNetworkMessage(move)) {
		lastSent = player;
		hasSentState = true;
	}
}

void SendLocalEscape(float escapeSeconds) {
	if (isEscapeSent) {
		return;
	}

	NetworkMessage escaped;
	escaped.type = MESSAGE_ESCAPED;
	escaped.player = static_cast<unsigned char>(GetLocalPlayer());
	escaped.value = static_cast<int>(escapeSeconds * 1000.0f);

	isEscapeSent = SendNetworkMessage(escaped);
}

LanGameEvent UpdateLanGame(LobbySettings& settings) {
	remote.hasUpdate = false;
	UpdateNetwork();

	// The messages that arrived before the connection closed are still read,
	// so a "bye" is noticed even when the other computer closed the connection right after it
	bool isConnectionLost = GetNetworkState() == NETWORK_ERROR;
	NetworkMessage message;

	while (ReceiveNetworkMessage(message)) {
		// Messages about our own player can't come from the other computer
		bool isFromOther = message.player != GetLocalPlayer();

		switch (message.type) {
		case MESSAGE_MOVE:
			if (isFromOther && message.seed <= DIRECTION_LEFT) {
				remote.hasUpdate = true;
				remote.x = message.x;
				remote.y = message.y;
				remote.moves = message.value;
				remote.facing = static_cast<Direction>(message.seed);
			}
			break;

		case MESSAGE_ESCAPED:
			if (isFromOther) {
				remote.hasUpdate = true;
				remote.hasEscaped = true;
				remote.escapeSeconds = message.value / 1000.0f;
			}
			break;

		case MESSAGE_START:
			// Only the host can start a rematch
			if (!IsHost() && IsValidMazeSize(message.value)) {
				settings.mode = message.player;
				settings.mazeSize = message.value;
				settings.seed = message.seed;
				ResetLanGame();
				return LAN_REMATCH;
			}
			break;

		case MESSAGE_BYE:
			Disconnect();
			return LAN_OTHER_LEFT;

		default:
			break;
		}
	}

	return isConnectionLost ? LAN_CONNECTION_LOST : LAN_NOTHING;
}

bool StartLanRematch(const LobbySettings& settings) {
	ResetLanGame();
	return StartLobbyGame(settings);
}

void LeaveLanGame() {
	ResetLanGame();
	LeaveLobby();
}
