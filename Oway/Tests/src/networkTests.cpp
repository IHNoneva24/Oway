#include "tests.h"
#include "testFramework.h"
#include "Network.h"
#include "Lobby.h"
#include <cstring>

// IP rules
static void TestIpValidation() {
	Check(IsValidIpAddress("192.168.1.15"), "192.168.1.15 is a valid IP");
	Check(IsValidIpAddress("10.0.0.1"), "10.0.0.1 is a valid IP");
	Check(IsValidIpAddress("255.255.255.255"), "255.255.255.255 is a valid IP");
	Check(!IsValidIpAddress("256.1.1.1"), "256.1.1.1 is rejected (a part is over 255)");
	Check(!IsValidIpAddress("1.2.3"), "1.2.3 is rejected (only 3 parts)");
	Check(!IsValidIpAddress("1.2.3.4.5"), "1.2.3.4.5 is rejected (5 parts)");
	Check(!IsValidIpAddress("01.2.3.4"), "01.2.3.4 is rejected (leading zero)");
	Check(!IsValidIpAddress("1..2.3"), "1..2.3 is rejected (empty part)");
	Check(!IsValidIpAddress("a.b.c.d"), "a.b.c.d is rejected (letters)");
	Check(!IsValidIpAddress(""), "an empty address is rejected");
	Check(!IsValidIpAddress(nullptr), "a missing address (nullptr) is rejected");
}

// Connection errors
static void TestConnectionErrors() {
	Check(InitNetwork(), "Winsock starts");
	Check(GetNetworkState() == NETWORK_OFF, "there is no connection at the start");

	NetworkMessage message;
	message.type = MESSAGE_MOVE;
	Check(!SendNetworkMessage(message), "nothing can be sent without a connection");
	Check(!ReceiveNetworkMessage(message), "nothing can be received without a connection");

	Check(!StartJoining("999.1.1.1", DEFAULT_PORT), "joining a wrong IP fails at once");
	Check(GetNetworkState() == NETWORK_ERROR && strcmp(GetNetworkError(), "Invalid IP address") == 0,
		"the error message says the IP is wrong");

	Disconnect();
	Check(GetNetworkState() == NETWORK_OFF, "Disconnect resets the error");

	LobbySettings settings;
	settings.mazeSize = 20;
	Check(!StartLobbyGame(settings), "a game can't be started in the lobby without a second player");

	char localIp[IP_TEXT_LENGTH] = "";
	Check(!GetLocalIpAddress(localIp, 4), "a too small buffer for the IP is rejected");

	ShutdownNetwork();
}

// Network tests
void RunNetworkTests() {
	BeginTestGroup("Network (BLL/Network, BLL/Lobby)");

	TestIpValidation();
	TestConnectionErrors();
}
