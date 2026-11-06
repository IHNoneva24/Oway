#pragma once

// Screens of the game
enum Screen {
	MAINMENU,
	SIZESELECT,
	GAME,
	WIN,
	TEAMGAME,
	TEAMWIN,
	MODESELECT,
	LOBBY,
	RACEGAME,
	RACEWIN,
	HELP,
	LEADERBOARD,
	EXITGAME
};

// Game modes, chosen in the main menu
enum GameMode {
	SINGLE_PLAYER,
	RACE,
	TEAM_PLAY
};
