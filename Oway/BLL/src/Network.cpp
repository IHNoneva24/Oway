#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Network.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

static const int MESSAGE_SIZE = 14;              // bytes of one message on the wire
static const int RECEIVE_BUFFER_SIZE = 4096;
static const unsigned long long CONNECT_TIMEOUT_MS = 5000;

static bool isWinsockStarted = false;
static SOCKET listenSocket = INVALID_SOCKET;     // only used by the host
static SOCKET peerSocket = INVALID_SOCKET;       // the connection to the other player
static NetworkState state = NETWORK_OFF;
static bool isHost = false;
static unsigned long long connectStartTime = 0;
static const char* lastError = "";

// Bytes received from TCP that are not a complete message yet
static char receiveBuffer[RECEIVE_BUFFER_SIZE];
static int receivedBytes = 0;

static void CloseSocket(SOCKET& socketToClose) {
	if (socketToClose != INVALID_SOCKET) {
		closesocket(socketToClose);
		socketToClose = INVALID_SOCKET;
	}
}

static void SetError(const char* message) {
	lastError = message;
	CloseSocket(peerSocket);
	CloseSocket(listenSocket);
	state = NETWORK_ERROR;
}

static bool MakeNonBlocking(SOCKET socketToChange) {
	u_long mode = 1;
	return ioctlsocket(socketToChange, FIONBIO, &mode) == 0;
}

// Messages are sent at once, without waiting to fill a bigger packet (lower delay for the moves)
static void DisableDelay(SOCKET socketToChange) {
	BOOL noDelay = TRUE;
	setsockopt(socketToChange, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&noDelay), sizeof(noDelay));
}

// Message <-> bytes. Numbers are sent in network byte order (big endian)
static void WriteMessage(const NetworkMessage& message, char* bytes) {
	u_short x = htons(static_cast<u_short>(message.x));
	u_short y = htons(static_cast<u_short>(message.y));
	u_long value = htonl(static_cast<u_long>(message.value));
	u_long seed = htonl(message.seed);

	bytes[0] = static_cast<char>(message.type);
	bytes[1] = static_cast<char>(message.player);
	memcpy(bytes + 2, &x, 2);
	memcpy(bytes + 4, &y, 2);
	memcpy(bytes + 6, &value, 4);
	memcpy(bytes + 10, &seed, 4);
}

static void ReadMessage(const char* bytes, NetworkMessage& message) {
	u_short x, y;
	u_long value, seed;

	memcpy(&x, bytes + 2, 2);
	memcpy(&y, bytes + 4, 2);
	memcpy(&value, bytes + 6, 4);
	memcpy(&seed, bytes + 10, 4);

	message.type = static_cast<MessageType>(static_cast<unsigned char>(bytes[0]));
	message.player = static_cast<unsigned char>(bytes[1]);
	message.x = static_cast<short>(ntohs(x));
	message.y = static_cast<short>(ntohs(y));
	message.value = static_cast<int>(ntohl(value));
	message.seed = ntohl(seed);
}

bool InitNetwork() {
	if (isWinsockStarted) {
		return true;
	}

	WSADATA data;
	isWinsockStarted = WSAStartup(MAKEWORD(2, 2), &data) == 0;
	return isWinsockStarted;
}

void ShutdownNetwork() {
	Disconnect();

	if (isWinsockStarted) {
		WSACleanup();
		isWinsockStarted = false;
	}
}

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
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
		SetError("The port is already in use");
		return false;
	}

	if (listen(listenSocket, 1) == SOCKET_ERROR) {
		SetError("Could not start the server");
		return false;
	}

	state = NETWORK_HOSTING;
	return true;
}

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
	address.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress, &address.sin_addr);

	peerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (peerSocket == INVALID_SOCKET || !MakeNonBlocking(peerSocket)) {
		SetError("Could not create the connection");
		return false;
	}

	// A non-blocking connect returns at once, UpdateNetwork() checks later if it worked
	if (connect(peerSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR
		&& WSAGetLastError() != WSAEWOULDBLOCK) {
		SetError("Could not reach the host");
		return false;
	}

	DisableDelay(peerSocket);
	connectStartTime = GetTickCount64();
	state = NETWORK_CONNECTING;
	return true;
}

// Host: takes the player who joins
static void AcceptPlayer() {
	SOCKET client = accept(listenSocket, nullptr, nullptr);

	if (client == INVALID_SOCKET) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			SetError("Error while waiting for a player");
		}

		return;
	}

	MakeNonBlocking(client);
	DisableDelay(client);
	peerSocket = client;

	// Only one player can join, so the server stops listening
	CloseSocket(listenSocket);
	state = NETWORK_CONNECTED;
}

// Client: checks if the connection to the host is ready
static void CheckConnecting() {
	fd_set writeSet;
	fd_set errorSet;
	FD_ZERO(&writeSet);
	FD_ZERO(&errorSet);
	FD_SET(peerSocket, &writeSet);
	FD_SET(peerSocket, &errorSet);

	timeval noWait = { 0, 0 };

	if (select(0, nullptr, &writeSet, &errorSet, &noWait) == SOCKET_ERROR || FD_ISSET(peerSocket, &errorSet)) {
		SetError("Could not reach the host");
		return;
	}

	if (FD_ISSET(peerSocket, &writeSet)) {
		state = NETWORK_CONNECTED;
		return;
	}

	if (GetTickCount64() - connectStartTime > CONNECT_TIMEOUT_MS) {
		SetError("The host did not answer");
	}
}

// Reads all bytes that arrived since the last frame
static void ReceiveBytes() {
	while (receivedBytes < RECEIVE_BUFFER_SIZE) {
		int count = recv(peerSocket, receiveBuffer + receivedBytes, RECEIVE_BUFFER_SIZE - receivedBytes, 0);

		if (count > 0) {
			receivedBytes += count;
			continue;
		}

		if (count == 0) {
			SetError("The other player left");
		}
		else if (WSAGetLastError() != WSAEWOULDBLOCK) {
			SetError("The connection was lost");
		}

		return;
	}
}

void UpdateNetwork() {
	switch (state) {
	case NETWORK_HOSTING:    AcceptPlayer();    break;
	case NETWORK_CONNECTING: CheckConnecting(); break;
	case NETWORK_CONNECTED:  ReceiveBytes();    break;
	default: break;
	}
}

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

NetworkState GetNetworkState() {
	return state;
}

bool IsHost() {
	return isHost;
}

bool SendNetworkMessage(const NetworkMessage& message) {
	if (state != NETWORK_CONNECTED) {
		return false;
	}

	char bytes[MESSAGE_SIZE];
	WriteMessage(message, bytes);

	int sent = 0;

	// A message is small, but send() can still take only a part of it
	while (sent < MESSAGE_SIZE) {
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

bool ReceiveNetworkMessage(NetworkMessage& message) {
	if (receivedBytes < MESSAGE_SIZE) {
		return false;
	}

	ReadMessage(receiveBuffer, message);

	// Moves the rest of the bytes to the start of the buffer
	receivedBytes -= MESSAGE_SIZE;
	memmove(receiveBuffer, receiveBuffer + MESSAGE_SIZE, receivedBytes);

	return true;
}

const char* GetNetworkError() {
	return lastError;
}

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
			// No leading zeros like "01" and every part is 0 - 255
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

bool GetLocalIpAddress(char* buffer, int bufferSize) {
	if (!isWinsockStarted || buffer == nullptr || bufferSize < IP_TEXT_LENGTH) {
		return false;
	}

	// Asks Windows which network card it would use to reach the internet. A UDP "connect" sends no data,
	// it only picks the route, so this finds the real LAN address and not a virtual adapter (VirtualBox, VPN)
	SOCKET probe = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (probe != INVALID_SOCKET) {
		sockaddr_in remote = {};
		remote.sin_family = AF_INET;
		remote.sin_port = htons(53);
		inet_pton(AF_INET, "8.8.8.8", &remote.sin_addr);

		sockaddr_in local = {};
		int localSize = sizeof(local);
		bool isRouteFound = connect(probe, reinterpret_cast<sockaddr*>(&remote), sizeof(remote)) == 0
			&& getsockname(probe, reinterpret_cast<sockaddr*>(&local), &localSize) == 0
			&& inet_ntop(AF_INET, &local.sin_addr, buffer, bufferSize) != nullptr;

		closesocket(probe);

		if (isRouteFound && strncmp(buffer, "0.", 2) != 0) {
			return true;
		}
	}

	// Without a route (no internet): the first address of this computer that is not 127.x.x.x
	char hostName[256];

	if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) {
		return false;
	}

	addrinfo hints = {};
	hints.ai_family = AF_INET;
	addrinfo* results = nullptr;

	if (getaddrinfo(hostName, nullptr, &hints, &results) != 0) {
		return false;
	}

	bool isFound = false;

	// The first address that is not 127.x.x.x (this computer only) is the LAN address
	for (addrinfo* result = results; result != nullptr && !isFound; result = result->ai_next) {
		sockaddr_in* address = reinterpret_cast<sockaddr_in*>(result->ai_addr);

		if (inet_ntop(AF_INET, &address->sin_addr, buffer, bufferSize) != nullptr && strncmp(buffer, "127.", 4) != 0) {
			isFound = true;
		}
	}

	freeaddrinfo(results);
	return isFound;
}
