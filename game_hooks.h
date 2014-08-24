#pragma once

#include "derandom.h"

#define STATE_PLAYING 0
#define STATE_INPUTLOCK_GENERIC 1
#define STATE_INPUTLOCK_LEVELSTART 2
#define STATE_GAMEOVER_HUD 30
#define STATE_MAINMENU 15
#define STATE_INTRO 14
#define STATE_TITLE 5
#define STATE_CHARSELECT 17
#define STATE_PLAYERSTATS 21
#define STATE_LOBBY 22
#define STATE_PAUSED 4
//OPT research other states

struct TimeInfo {
	unsigned minutes;
	unsigned seconds;
	double milliseconds;

	TimeInfo() : minutes(0), seconds(0), milliseconds(0.0) {}

	double total_ms() {
		return milliseconds + seconds*1000 + minutes*60*1000;
	}
};

class GameHooks {
private:
	bool is_valid;

	bool have_steamid;
	std::string steamid;

	Address game_state_offset;
	Address g_CurrentGamePtr;
	Address game_goldcount_offset;
	Address game_timer_offset;
	Address level_timer_offset;
	
	Address player_bomb_offs;
	Address player_health_offs;
	Address player_rope_offs;
	unsigned player_struct_size;

	std::shared_ptr<DerandomizePatch> dp;
	std::shared_ptr<Spelunky> spel;

	bool discover_steamid();
	bool discover_gold_count();
	bool discover_timers();
	bool discover_game_state();
	bool discover_player_data();

public:
	GameHooks(std::shared_ptr<Spelunky> spel, std::shared_ptr<DerandomizePatch> dp);
	int game_state();
	int current_level();
	
	//first player_id = 0
	int bombs(int player_id);
	int health(int player_id);
	int ropes(int player_id);

	unsigned gold_count();
	TimeInfo game_timer();
	TimeInfo level_timer();
	bool valid();
	std::string steam_id();
};