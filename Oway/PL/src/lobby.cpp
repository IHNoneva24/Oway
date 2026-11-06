#include "screens.h"
#include "ui.h"
#include "Lobby.h"
#include "lanSession.h"
#include "Maze.h"
#include <ctime>

extern int screenW;
extern int screenH;
extern GameMode gameMode;

// What the lobby shows at the moment
enum LobbyView {
	VIEW_CHOOSE,   // host or join
	VIEW_HOST,     // waiting for a player / choosing the size
	VIEW_JOIN,     // typing the host's IP
	VIEW_WAITING   // client: connecting or waiting for the host to start
};

static LobbyView view = VIEW_CHOOSE;
static char ipInput[IP_TEXT_LENGTH] = "";
static char localIp[IP_TEXT_LENGTH] = "";
static int mazeSize = 20;
static const char* message = "";   // error or info text under the panel
static bool isMessageError = false;

static void ShowMessage(const char* text, bool isError) {
	message = text;
	isMessageError = isError;
}

static void ResetLobbyScreen() {
	view = VIEW_CHOOSE;
	ShowMessage("", false);
}

static const char* GetModeName() {
	return gameMode == TEAM_PLAY ? "TEAM PLAY" : "RACE";
}

// One row of the player list
static void DrawPlayerRow(float y, const char* name, const char* status, Color statusColor) {
	float left = screenW / 2.0f - Scaled(330);
	int fontSize = static_cast<int>(Scaled(32));

	DrawText(name, static_cast<int>(left), static_cast<int>(y), fontSize, RAYWHITE);
	DrawText(status, static_cast<int>(left + Scaled(380)), static_cast<int>(y), fontSize, statusColor);
}

static void DrawPlayerList(float y, bool isOtherConnected) {
	bool isLocalHost = IsHost();

	DrawPlayerRow(y, isLocalHost ? "Player 1 (you)" : "Player 1", "host", COLOR_ACCENT);
	DrawPlayerRow(y + Scaled(50), isLocalHost ? "Player 2" : "Player 2 (you)",
		isOtherConnected ? "ready" : "waiting...", isOtherConnected ? COLOR_ACCENT : COLOR_TEXT_DIM);
}

static void DrawChooseView(Screen& currentScreen, float y) {
	DrawCenteredText("Play with a friend on the same network", y, Scaled(30), COLOR_TEXT_DIM);

	if (DrawCenteredButton(y + Scaled(90), "Host game")) {
		if (HostLobby()) {
			if (!GetLocalIpAddress(localIp, IP_TEXT_LENGTH)) {
				TextCopy(localIp, "unknown");
			}

			view = VIEW_HOST;
			ShowMessage("", false);
		}
		else {
			ShowMessage(GetNetworkError(), true);
		}
	}

	if (DrawCenteredButton(y + Scaled(185), "Join game")) {
		view = VIEW_JOIN;
		ShowMessage("", false);
	}

	if (DrawCenteredButton(y + Scaled(280), "Back") || IsKeyPressed(KEY_ESCAPE)) {
		ResetLobbyScreen();
		currentScreen = MODESELECT;
	}
}

static void DrawHostView(Screen& currentScreen, float y) {
	DrawCenteredText("Your IP address:", y, Scaled(28), COLOR_TEXT_DIM);
	DrawCenteredText(localIp, y + Scaled(40), Scaled(56), COLOR_ACCENT);
	DrawCenteredText("Tell it to the other player", y + Scaled(105), Scaled(24), COLOR_TEXT_DIM);

	DrawPlayerList(y + Scaled(160), IsLobbyReady());

	// Maze size, chosen by the host
	float sizeY = y + Scaled(290);
	float smallButton = Scaled(70);
	DrawCenteredText(TextFormat("Maze: %d x %d", mazeSize, mazeSize), sizeY + Scaled(16), Scaled(36), RAYWHITE);

	if (DrawButton({ screenW / 2.0f - Scaled(260), sizeY, smallButton, smallButton }, "-") && mazeSize > MIN_MAZE_SIZE) {
		mazeSize--;
	}

	if (DrawButton({ screenW / 2.0f + Scaled(190), sizeY, smallButton, smallButton }, "+") && mazeSize < MAX_MAZE_SIZE) {
		mazeSize++;
	}

	const char* startText = IsLobbyReady() ? "Start" : "Waiting for player...";

	if (DrawCenteredButton(sizeY + Scaled(110), startText) && IsLobbyReady()) {
		LobbySettings settings;
		settings.mode = gameMode;
		settings.mazeSize = mazeSize;
		settings.seed = static_cast<unsigned int>(time(nullptr));

		if (StartLobbyGame(settings) && StartLanSession(settings, currentScreen)) {
			ResetLobbyScreen();
		}
		else {
			ShowMessage("Could not start the game", true);
		}
	}

	if (DrawCenteredButton(sizeY + Scaled(205), "Cancel") || IsKeyPressed(KEY_ESCAPE)) {
		LeaveLobby();
		ResetLobbyScreen();
	}
}

static void DrawJoinView(float y) {
	DrawCenteredText("Type the IP address of the host:", y, Scaled(28), COLOR_TEXT_DIM);

	if (UpdateFilteredTextInput(ipInput, IP_TEXT_LENGTH - 1, "0123456789.")) {
		ShowMessage("", false);
	}

	float boxWidth = Scaled(520);
	DrawTextBox({ screenW / 2.0f - boxWidth / 2, y + Scaled(50), boxWidth, Scaled(90) }, ipInput, Scaled(52),
		isMessageError);

	if (DrawCenteredButton(y + Scaled(180), "Connect") || IsKeyPressed(KEY_ENTER)) {
		if (!IsValidIpAddress(ipInput)) {
			ShowMessage("Enter an address like 192.168.1.15", true);
		}
		else if (JoinLobby(ipInput)) {
			view = VIEW_WAITING;
			ShowMessage("", false);
		}
		else {
			ShowMessage(GetNetworkError(), true);
		}
	}

	if (DrawCenteredButton(y + Scaled(275), "Back") || IsKeyPressed(KEY_ESCAPE)) {
		ResetLobbyScreen();
	}
}

static void DrawWaitingView(float y) {
	bool isConnected = IsLobbyReady();
	const char* status = isConnected ? "Waiting for the host to start..." : TextFormat("Connecting to %s...", ipInput);

	DrawCenteredText(status, y, Scaled(32), COLOR_ACCENT);
	DrawPlayerList(y + Scaled(80), isConnected);

	if (DrawCenteredButton(y + Scaled(250), "Leave") || IsKeyPressed(KEY_ESCAPE)) {
		LeaveLobby();
		ResetLobbyScreen();
	}
}

// The host keeps the lobby open for a new player, the client goes back to the IP input
static void WaitForNextPlayer() {
	if (view == VIEW_HOST) {
		if (!HostLobby()) {
			view = VIEW_CHOOSE;
		}
	}
	else {
		LeaveLobby();
		view = VIEW_JOIN;
	}
}

// Reacts to what happened on the network this frame
static void HandleLobbyEvents(Screen& currentScreen) {
	LobbySettings settings;
	LobbyEvent event = UpdateLobby(settings);

	switch (event) {
	case LOBBY_PLAYER_JOINED:
		ShowMessage("Player 2 joined!", false);
		break;

	case LOBBY_CONNECTED:
		ShowMessage("Connected to the host", false);
		break;

	case LOBBY_GAME_STARTED:
		// The game mode comes from the host, so both play the same mode
		gameMode = static_cast<GameMode>(settings.mode);

		if (StartLanSession(settings, currentScreen)) {
			ResetLobbyScreen();
		}
		else {
			LeaveLobby();
			ShowMessage("Could not start the game", true);
		}
		break;

	case LOBBY_WRONG_VERSION:
		ShowMessage("The other player has a different version of the game", true);
		WaitForNextPlayer();
		break;

	case LOBBY_PLAYER_LEFT:
		ShowMessage("The other player left", true);
		WaitForNextPlayer();
		break;

	case LOBBY_NOTHING:
		break;
	}

	// Errors like "Could not reach the host" send the client back to the IP input
	if (GetNetworkState() == NETWORK_ERROR) {
		ShowMessage(GetNetworkError(), true);
		LeaveLobby();
		view = view == VIEW_WAITING ? VIEW_JOIN : VIEW_CHOOSE;
	}
}

void DrawLobby(Screen& currentScreen) {
	DrawDungeonBackground();

	float panelWidth = Scaled(900);
	float panelHeight = Scaled(820);
	float panelY = screenH / 2.0f - panelHeight / 2;
	DrawPanel({ screenW / 2.0f - panelWidth / 2, panelY, panelWidth, panelHeight });

	float y = panelY + Scaled(40);
	DrawCenteredText(TextFormat("%s - LAN", GetModeName()), y, Scaled(60), COLOR_ACCENT);
	y += Scaled(120);

	if (view != VIEW_CHOOSE && view != VIEW_JOIN) {
		HandleLobbyEvents(currentScreen);
	}

	switch (view) {
	case VIEW_CHOOSE:  DrawChooseView(currentScreen, y); break;
	case VIEW_HOST:    DrawHostView(currentScreen, y);   break;
	case VIEW_JOIN:    DrawJoinView(y);                  break;
	case VIEW_WAITING: DrawWaitingView(y);               break;
	}

	if (message[0] != '\0') {
		DrawCenteredText(message, panelY + panelHeight - Scaled(60), Scaled(28), isMessageError ? COLOR_ERROR : COLOR_ACCENT);
	}
}
