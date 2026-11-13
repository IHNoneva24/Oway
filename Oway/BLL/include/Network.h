#pragma once

const unsigned short DEFAULT_PORT = 52025;
const int IP_TEXT_LENGTH = 16;

// Connection states
enum NetworkState {
	NETWORK_OFF,
	NETWORK_HOSTING,
	NETWORK_CONNECTING,
	NETWORK_CONNECTED,
	NETWORK_ERROR
};

// Message types
enum MessageType : unsigned char {
	MESSAGE_HELLO = 1,
	MESSAGE_START,
	MESSAGE_MOVE,
	MESSAGE_ESCAPED,
	MESSAGE_BYE
};

// Network message
struct NetworkMessage {
	MessageType type = MESSAGE_HELLO;
	unsigned char player = 0;
	short x = 0;
	short y = 0;
	int value = 0;
	unsigned int seed = 0;
};

const int PROTOCOL_VERSION = 1;

// Start network
bool InitNetwork();

// Stop network
void ShutdownNetwork();

// Host game
bool StartHosting(unsigned short port);

// Join game
bool StartJoining(const char* ipAddress, unsigned short port);

// Update network
void UpdateNetwork();

// Disconnect
void Disconnect();

// Connection state
NetworkState GetNetworkState();

// Host check
bool IsHost();

// Send message
bool SendNetworkMessage(const NetworkMessage& message);

// Receive message
bool ReceiveNetworkMessage(NetworkMessage& message);

// Last error
const char* GetNetworkError();

// IP check
bool IsValidIpAddress(const char* text);

// Local IP
bool GetLocalIpAddress(char* buffer, int bufferSize);
