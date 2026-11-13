// smaller windows.h
#define WIN32_LEAN_AND_MEAN
// no min/max macros
#define NOMINMAX
#include "Network.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>

// link Winsock
#pragma comment(lib, "ws2_32.lib")

static const int MESSAGE_SIZE = 14;
static const int RECEIVE_BUFFER_SIZE = 4096;
static const unsigned long long CONNECT_TIMEOUT_MS = 5000;

static bool isWinsockStarted = false;
static SOCKET listenSocket = INVALID_SOCKET;
static SOCKET peerSocket = INVALID_SOCKET;
static NetworkState state = NETWORK_OFF;
static bool isHost = false;
static unsigned long long connectStartTime = 0;
static const char* lastError = "";

static char receiveBuffer[RECEIVE_BUFFER_SIZE];
static int receivedBytes = 0;

// Close socket
static void CloseSocket(SOCKET& socketToClose) {
	if (socketToClose != INVALID_SOCKET) {
		closesocket(socketToClose);
		socketToClose = INVALID_SOCKET;
	}
}

// Set error
static void SetError(const char* message) {
	lastError = message;
	CloseSocket(peerSocket);
	CloseSocket(listenSocket);
	state = NETWORK_ERROR;
}

// Non-blocking socket
static bool MakeNonBlocking(SOCKET socketToChange) {
	u_long mode = 1;
	// non-blocking mode
	return ioctlsocket(socketToChange, FIONBIO, &mode) == 0;
}

// Disable delay
static void DisableDelay(SOCKET socketToChange) {
	BOOL noDelay = TRUE;
	// send at once
	setsockopt(socketToChange, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&noDelay), sizeof(noDelay));
}

// Pack message
static void WriteMessage(const NetworkMessage& message, char* bytes) {
	// network byte order
	u_short x = htons(static_cast<u_short>(message.x));
	u_short y = htons(static_cast<u_short>(message.y));
	// network byte order
	u_long value = htonl(static_cast<u_long>(message.value));
	u_long seed = htonl(message.seed);

	bytes[0] = static_cast<char>(message.type);
	bytes[1] = static_cast<char>(message.player);
	// copy bytes
	memcpy(bytes + 2, &x, 2);
	memcpy(bytes + 4, &y, 2);
	// copy bytes
	memcpy(bytes + 6, &value, 4);
	memcpy(bytes + 10, &seed, 4);
}

// Unpack message
static void ReadMessage(const char* bytes, NetworkMessage& message) {
	u_short x, y;
	u_long value, seed;

	// copy bytes
	memcpy(&x, bytes + 2, 2);
	memcpy(&y, bytes + 4, 2);
	// copy bytes
	memcpy(&value, bytes + 6, 4);
	memcpy(&seed, bytes + 10, 4);

	message.type = static_cast<MessageType>(static_cast<unsigned char>(bytes[0]));
	message.player = static_cast<unsigned char>(bytes[1]);
	// host byte order
	message.x = static_cast<short>(ntohs(x));
	message.y = static_cast<short>(ntohs(y));
	// host byte order
	message.value = static_cast<int>(ntohl(value));
	message.seed = ntohl(seed);
}

// Start network
bool InitNetwork() {
	if (isWinsockStarted) {
		return true;
	}

	WSADATA data;
	// start Winsock
	isWinsockStarted = WSAStartup(MAKEWORD(2, 2), &data) == 0;
	return isWinsockStarted;
}

// Stop network
void ShutdownNetwork() {
	Disconnect();

	if (isWinsockStarted) {
		// stop Winsock
		WSACleanup();
		isWinsockStarted = false;
	}
}

// Host game
bool StartHosting(unsigned short port) {
	Disconnect();
	isHost = true;

	if (!isWinsockStarted) {
		SetError("Network is not available");
		return false;
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSocket == INVALID_SOCKET || !MakeNonBlocking(listenSocket)) {
		SetError("Could not create the server");
		return false;
	}

	sockaddr_in address = {};
	address.sin_family = AF_INET;
	// network byte order
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	// reserve port
	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
		SetError("The port is already in use");
		return false;
	}

	// wait for players
	if (listen(listenSocket, 1) == SOCKET_ERROR) {
		SetError("Could not start the server");
		return false;
	}

	state = NETWORK_HOSTING;
	return true;
}

// Join game
bool StartJoining(const char* ipAddress, unsigned short port) {
	Disconnect();
	isHost = false;

	if (!isWinsockStarted) {
		SetError("Network is not available");
		return false;
	}

	if (!IsValidIpAddress(ipAddress)) {
		SetError("Invalid IP address");
		return false;
	}

	sockaddr_in address = {};
	address.sin_family = AF_INET;
	// network byte order
	address.sin_port = htons(port);
	// text to IP
	inet_pton(AF_INET, ipAddress, &address.sin_addr);

	peerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (peerSocket == INVALID_SOCKET || !MakeNonBlocking(peerSocket)) {
		SetError("Could not create the connection");
		return false;
	}

	// start connecting
	if (connect(peerSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR
		// real error
		&& WSAGetLastError() != WSAEWOULDBLOCK) {
		SetError("Could not reach the host");
		return false;
	}

	DisableDelay(peerSocket);
	connectStartTime = GetTickCount64();
	state = NETWORK_CONNECTING;
	return true;
}

// Accept player
static void AcceptPlayer() {
	// take player
	SOCKET client = accept(listenSocket, nullptr, nullptr);

	if (client == INVALID_SOCKET) {
		// real error
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			SetError("Error while waiting for a player");
		}

		return;
	}

	MakeNonBlocking(client);
	DisableDelay(client);
	peerSocket = client;

	CloseSocket(listenSocket);
	state = NETWORK_CONNECTED;
}

// Check connection
static void CheckConnecting() {
	fd_set writeSet;
	fd_set errorSet;
	// socket sets
	FD_ZERO(&writeSet);
	FD_ZERO(&errorSet);
	FD_SET(peerSocket, &writeSet);
	FD_SET(peerSocket, &errorSet);

	timeval noWait = { 0, 0 };

	// check socket
	if (select(0, nullptr, &writeSet, &errorSet, &noWait) == SOCKET_ERROR || FD_ISSET(peerSocket, &errorSet)) {
		SetError("Could not reach the host");
		return;
	}

	if (FD_ISSET(peerSocket, &writeSet)) {
		state = NETWORK_CONNECTED;
		return;
	}

	// connect timeout
	if (GetTickCount64() - connectStartTime > CONNECT_TIMEOUT_MS) {
		SetError("The host did not answer");
	}
}

// Receive bytes
static void ReceiveBytes() {
	while (receivedBytes < RECEIVE_BUFFER_SIZE) {
		// read bytes
		int count = recv(peerSocket, receiveBuffer + receivedBytes, RECEIVE_BUFFER_SIZE - receivedBytes, 0);

		if (count > 0) {
			receivedBytes += count;
			continue;
		}

		if (count == 0) {
			SetError("The other player left");
		}
		// real error
		else if (WSAGetLastError() != WSAEWOULDBLOCK) {
			SetError("The connection was lost");
		}

		return;
	}
}

// Update network
void UpdateNetwork() {
	switch (state) {
	case NETWORK_HOSTING:    AcceptPlayer();    break;
	case NETWORK_CONNECTING: CheckConnecting(); break;
	case NETWORK_CONNECTED:  ReceiveBytes();    break;
	default: break;
	}
}

// Disconnect
void Disconnect() {
	if (state == NETWORK_CONNECTED) {
		NetworkMessage bye;
		bye.type = MESSAGE_BYE;
		SendNetworkMessage(bye);
	}

	CloseSocket(peerSocket);
	CloseSocket(listenSocket);
	receivedBytes = 0;
	lastError = "";
	state = NETWORK_OFF;
}

// Connection state
NetworkState GetNetworkState() {
	return state;
}

// Host check
bool IsHost() {
	return isHost;
}

// Send message
bool SendNetworkMessage(const NetworkMessage& message) {
	if (state != NETWORK_CONNECTED) {
		return false;
	}

	char bytes[MESSAGE_SIZE];
	WriteMessage(message, bytes);

	int sent = 0;

	while (sent < MESSAGE_SIZE) {
		// send bytes
		int count = send(peerSocket, bytes + sent, MESSAGE_SIZE - sent, 0);

		if (count == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) {
				continue;
			}

			SetError("The connection was lost");
			return false;
		}

		sent += count;
	}

	return true;
}

// Receive message
bool ReceiveNetworkMessage(NetworkMessage& message) {
	if (receivedBytes < MESSAGE_SIZE) {
		return false;
	}

	ReadMessage(receiveBuffer, message);

	receivedBytes -= MESSAGE_SIZE;
	// shift buffer
	memmove(receiveBuffer, receiveBuffer + MESSAGE_SIZE, receivedBytes);

	return true;
}

// Last error
const char* GetNetworkError() {
	return lastError;
}

// IP check
bool IsValidIpAddress(const char* text) {
	if (text == nullptr) {
		return false;
	}

	int parts = 0;
	int value = 0;
	int digits = 0;

	for (int i = 0; ; i++) {
		char character = text[i];

		if (character >= '0' && character <= '9') {
			if (digits > 0 && value == 0) {
				return false;
			}

			value = value * 10 + (character - '0');
			digits++;

			if (value > 255) {
				return false;
			}
		}
		else if (character == '.' || character == '\0') {
			if (digits == 0) {
				return false;
			}

			parts++;
			value = 0;
			digits = 0;

			if (character == '\0') {
				return parts == 4;
			}
		}
		else {
			return false;
		}
	}
}

// Local IP
bool GetLocalIpAddress(char* buffer, int bufferSize) {
	if (!isWinsockStarted || buffer == nullptr || bufferSize < IP_TEXT_LENGTH) {
		return false;
	}

	// UDP probe
	SOCKET probe = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (probe != INVALID_SOCKET) {
		sockaddr_in remote = {};
		remote.sin_family = AF_INET;
		// network byte order
		remote.sin_port = htons(53);
		// text to IP
		inet_pton(AF_INET, "8.8.8.8", &remote.sin_addr);

		sockaddr_in local = {};
		int localSize = sizeof(local);
		// start connecting
		bool isRouteFound = connect(probe, reinterpret_cast<sockaddr*>(&remote), sizeof(remote)) == 0
			// local address
			&& getsockname(probe, reinterpret_cast<sockaddr*>(&local), &localSize) == 0
			// IP to text
			&& inet_ntop(AF_INET, &local.sin_addr, buffer, bufferSize) != nullptr;

		closesocket(probe);

		if (isRouteFound && strncmp(buffer, "0.", 2) != 0) {
			return true;
		}
	}

	char hostName[256];

	// computer name
	if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) {
		return false;
	}

	addrinfo hints = {};
	hints.ai_family = AF_INET;
	addrinfo* results = nullptr;

	// host addresses
	if (getaddrinfo(hostName, nullptr, &hints, &results) != 0) {
		return false;
	}

	bool isFound = false;

	for (addrinfo* result = results; result != nullptr && !isFound; result = result->ai_next) {
		// raw cast
		sockaddr_in* address = reinterpret_cast<sockaddr_in*>(result->ai_addr);

		// IP to text
		if (inet_ntop(AF_INET, &address->sin_addr, buffer, bufferSize) != nullptr && strncmp(buffer, "127.", 4) != 0) {
			isFound = true;
		}
	}

	freeaddrinfo(results);
	return isFound;
}
