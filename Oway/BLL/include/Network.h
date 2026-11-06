#pragma once

// LAN networking with Winsock (TCP). One player hosts, the other one joins with the host's IP.
// Everything is non-blocking, so the game never freezes while waiting for the network.
// Winsock is only included in Network.cpp, so its names never collide with raylib

const unsigned short DEFAULT_PORT = 52025;
const int IP_TEXT_LENGTH = 16; // "255.255.255.255" + '\0'

enum NetworkState {
	NETWORK_OFF,
	NETWORK_HOSTING,     // server is waiting for a player to join
	NETWORK_CONNECTING,  // client is trying to reach the host
	NETWORK_CONNECTED,   // both sides can send messages
	NETWORK_ERROR
};

enum MessageType : unsigned char {
	MESSAGE_HELLO = 1,   // client -> host after connecting (value = protocol version)
	MESSAGE_START,       // host -> client: start a game (seed, value = maze size, player = game mode)
	MESSAGE_MOVE,        // a player moved (player, x, y, value = moves)
	MESSAGE_ESCAPED,     // a player reached the exit (player, value = time in milliseconds)
	MESSAGE_BYE          // the other side leaves the game
};

// Every message has the same fixed size, so reading them from the TCP stream is simple
struct NetworkMessage {
	MessageType type = MESSAGE_HELLO;
	unsigned char player = 0;
	short x = 0;
	short y = 0;
	int value = 0;
	unsigned int seed = 0;
};

const int PROTOCOL_VERSION = 1;

// Starts Winsock once at the start of the game. Returns false if the network can't be used
bool InitNetwork();

// Closes all connections and stops Winsock
void ShutdownNetwork();

// Opens a server on the port and waits for one player. Returns false on error (e.g. port in use)
bool StartHosting(unsigned short port);

// Starts connecting to a host. The result is known later from GetNetworkState()
bool StartJoining(const char* ipAddress, unsigned short port);

// Accepts the joining player / finishes connecting / notices a lost connection. Call once per frame
void UpdateNetwork();

// Closes the connection (and the server) and goes back to NETWORK_OFF
void Disconnect();

NetworkState GetNetworkState();

bool IsHost();

// Sends one message. Returns false if there is no connection
bool SendNetworkMessage(const NetworkMessage& message);

// Takes the next received message. Returns false if there is none
bool ReceiveNetworkMessage(NetworkMessage& message);

// Short description of the last error, for the interface
const char* GetNetworkError();

// Checks that the text is an IPv4 address like 192.168.1.15
bool IsValidIpAddress(const char* text);

// Writes this computer's LAN IP address (to show to the other player). Returns false if not found
bool GetLocalIpAddress(char* buffer, int bufferSize);
