#include "patches.h"
#include "game_hooks.h"
#include "spelunky.h"

#include <Windows.h>
#include <string>


#ifdef DEBUG_MODE
#define DISCOVERY_FUNC(FUNC) if(!FUNC()) {std::cout << "GameHooks::" << #FUNC << " failed, hooks invalidated." << std::endl; is_valid = false;}
#else
#define DISCOVERY_FUNC(FUNC) if(!FUNC()) {is_valid = false;}
#endif

GameHooks::GameHooks(std::shared_ptr<Spelunky> spel, std::shared_ptr<DerandomizePatch> dp) : 
	is_valid(true),
	spel(spel), 
	dp(dp),
	have_steamid(false)
{
	g_CurrentGamePtr = dp->game_ptr();
	if(g_CurrentGamePtr == 0x0) {
		is_valid = false;
		return;
	}
	
	DISCOVERY_FUNC(discover_game_state);
	DISCOVERY_FUNC(discover_timers);
	DISCOVERY_FUNC(discover_gold_count);
	DISCOVERY_FUNC(discover_steamid);
	DISCOVERY_FUNC(discover_player_data);
}


BYTE game_state_find[] = {0xBB, 0x0F, 0x00, 0x00, 0x00, 0x3B, 0xC3, 0x75, 0xFF, 0x8B, 0x7E, 0xFF, 0xC7, 0x46, 0xFF, 0x1B, 0x00, 0x00, 0x00, 0x89, 0x5E, 0xFF, 0xE8};
std::string game_state_mask = "xxxxxxxx.xx.xx.xxxxxx.x";

bool GameHooks::discover_game_state() {
	game_state_offset = spel->get_stored_hook("game_state_offset");
	if(game_state_offset == 0x0) 
	{
		Address game_state_ptr = spel->find_mem(game_state_find, game_state_mask);
		if(game_state_ptr == 0x0) {
			return false;
		}
		game_state_ptr += 0x15;

		BYTE byte_state_offset;
		spel->read_mem(game_state_ptr, &byte_state_offset, sizeof(BYTE));

		game_state_offset = (Address)byte_state_offset;
		spel->store_hook("game_state_offset", game_state_offset);

#ifdef DEBUG_MODE
		std::cout << "game_state_offset -> " << std::setbase(16) << (Address)game_state_offset << std::endl;
#endif
	}

	return true;
}

BYTE game_timer_offset_find[] = {0xD8, 0xC2, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xD8, 0xD9, 0xDF, 0xE0, 0x84, 0xFF, 0x75, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01};
std::string game_timer_offset_mask = "xxx.....xxxxx.x.x.....x";

bool GameHooks::discover_timers() {
	game_timer_offset = spel->get_stored_hook("game_timer_offset");
	if(game_timer_offset == 0x0) 
	{
		Address game_timer_offset_ptr = spel->find_mem(game_timer_offset_find, game_timer_offset_mask);
		if(game_timer_offset_ptr == 0x0) {
			return false;
		}
		game_timer_offset_ptr += 4;

		spel->read_mem(game_timer_offset_ptr, &game_timer_offset, sizeof(Address));
		game_timer_offset -= 8; //this points to milliseconds when read, we want to start from minutes

#ifdef DEBUG_MODE
		std::cout << "game_timer_offset -> " << std::setbase(16) << game_timer_offset << std::endl;
#endif
		spel->store_hook("game_timer_offset", game_timer_offset);
	}

	if(game_timer_offset != 0x0) {
		level_timer_offset = game_timer_offset + 16;
	}

	return true;
}



BYTE game_goldcount_offset_find[] = {0x8B, 0x97, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x50, 0xFF, 0x8B, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x8B, 0x0D, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x81, 0xFF, 0xFF, 0xFF, 0xFF, 0x8B, 0x83, 0x00, 0x00, 0x00, 0x00};
std::string game_goldcount_offset_mask = "xx....xx.xx....xx....xx....xx....";

bool GameHooks::discover_gold_count() {
	game_goldcount_offset = spel->get_stored_hook("game_goldcount_offset");
	if(game_goldcount_offset == 0x0) 
	{
		Address game_goldcount_offset_ptr = spel->find_mem(game_goldcount_offset_find, game_goldcount_offset_mask);
		if(game_goldcount_offset_ptr == 0x0) {
			return false;
		}
		game_goldcount_offset_ptr += 0x17;

		spel->read_mem(game_goldcount_offset_ptr, &game_goldcount_offset, sizeof(Address));

#ifdef DEBUG_MODE
		std::cout << "gold_count_offset -> " << std::setbase(16) << game_goldcount_offset << std::endl;
#endif
		spel->store_hook("game_goldcount_offset", game_goldcount_offset);
	}

	return true;
}


#define STEAMID_MEM_LENGTH 18
BYTE steamid_find[] = "STEAMID=";
std::string steamid_mask = "xxxxxxxx";

#ifdef DEBUG_MODE
bool debugged_steamid = false;
#endif

bool valid_steamid(char id[STEAMID_MEM_LENGTH]) {
	for(int i = 0; i < STEAMID_MEM_LENGTH-1; i++) {
		char ch = id[i];
		if(ch < '0' || ch > '9') {
			return false;
		}
	}

	id[STEAMID_MEM_LENGTH] = 0x0;
	return true;
}

bool GameHooks::discover_steamid() 
{
	char id[STEAMID_MEM_LENGTH];
	Address start = 0x0;
	while(true) {
		start = spel->find_mem(steamid_find, steamid_mask, start);
		if(start == 0x0) {
			have_steamid = false;
			return false;
		}

		spel->read_mem(start+sizeof(steamid_find)-1, &id, sizeof(id));
		if(valid_steamid(id)) {
			break;
		}

		start++;
	}

	this->steamid = std::string(id);
	have_steamid = true;

#ifdef DEBUG_MODE
	std::cout << "Discovered steamid " << this->steamid << " at " << start << std::endl;
#endif

	return true;
}

BYTE playerdata_find[] = {0x33, 0xCC, 0x8B, 0xCC, 0x69, 0xCC, 0xAA, 0xAA, 0xAA, 0xAA, 0xCC, 0x04,
	0x00, 0x00, 0x00, 0x89, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x89, 0xCC, 0xCC, 0xAA, 0xAA, 0xAA, 0xAA, 0x89};
std::string playerdata_mask = "x.x.x......xxxxx......x......x";

#ifdef DEBUG_MODE
bool debugged_player_data = false;
#endif

bool GameHooks::discover_player_data() 
{
	Address playerdata_addr = spel->get_stored_hook("pdat");
	if(playerdata_addr == 0x0) {
		playerdata_addr = spel->find_mem(playerdata_find, playerdata_mask);
		if(playerdata_addr == 0x0) {
			return false;
		}
		spel->store_hook("pdat", playerdata_addr);
	}
	
	spel->read_mem(playerdata_addr+6, &player_struct_size, sizeof(unsigned));
	spel->read_mem(playerdata_addr+32, &player_health_offs, sizeof(Address));
	spel->read_mem(playerdata_addr+39, &player_bomb_offs, sizeof(Address));
	spel->read_mem(playerdata_addr+46, &player_rope_offs, sizeof(Address));

#ifdef DEBUG_MODE
	if(!debugged_player_data) {
		debugged_player_data = true;
		std::cout << "Size of a player structure = " << std::setbase(16) << player_struct_size << std::endl;
		std::cout << "Game health offset = " << std::setbase(16) << player_health_offs << std::endl;
		std::cout << "Game bomb offset = " << std::setbase(16) << player_bomb_offs << std::endl;
		std::cout << "Game rope offset = " << std::setbase(16) << player_rope_offs << std::endl;
	}
#endif

	return true;
}

int GameHooks::bombs(int player_id) {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	int bomb_count = 0;
	spel->read_mem(game + player_bomb_offs + player_struct_size*player_id, &bomb_count, sizeof(int));

	return bomb_count;
}

int GameHooks::ropes(int player_id) {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	int ropes_count = 0;
	spel->read_mem(game + player_rope_offs + player_struct_size*player_id, &ropes_count, sizeof(int));

	return ropes_count;
}

int GameHooks::health(int player_id) {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	int health_count = 0;
	spel->read_mem(game + player_health_offs + player_struct_size*player_id, &health_count, sizeof(int));

	return health_count;
}


int GameHooks::game_state() {
	Address game;
	spel->read_mem(g_CurrentGamePtr, &game, sizeof(Address));

	int state;
	spel->read_mem(game+game_state_offset, &state, sizeof(unsigned));

	return state;
}

unsigned GameHooks::gold_count() {
	Address game;
	spel->read_mem(g_CurrentGamePtr, &game, sizeof(Address));

	unsigned gold_count;
	spel->read_mem(game+game_goldcount_offset, &gold_count, sizeof(unsigned));

	return gold_count;
}

TimeInfo GameHooks::game_timer() {
	Address game;
	spel->read_mem(g_CurrentGamePtr, &game, sizeof(Address));

	TimeInfo ti;
	spel->read_mem(game+game_timer_offset, &ti, sizeof(TimeInfo));
	
	return ti;
}

TimeInfo GameHooks::level_timer() {
	Address game;
	spel->read_mem(g_CurrentGamePtr, &game, sizeof(Address));

	TimeInfo ti;
	spel->read_mem(game+level_timer_offset, &ti, sizeof(TimeInfo));
	return ti;
}

int GameHooks::current_level() {
	return dp->current_level();
}

bool GameHooks::valid() {
	return is_valid;
}

std::string GameHooks::steam_id() {
	if(have_steamid) {
		//prevent steam id from being overwritten
		discover_steamid();
		return steamid;
	}
	else
		return "";
}