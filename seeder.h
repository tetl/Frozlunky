#pragma once

#include "patches.h"
#include "derandom.h"
//#include "second_rand.h"
#include <boost/algorithm/string.hpp>
#include <memory>
#include <string>
#include <vector>
#include <random>

class Seeder {
public:
	typedef size_t Seed;

private:
	std::shared_ptr<DerandomizePatch> dp;
	std::vector<std::string> stack;

	Seed current_seed;
	std::string* current_str_seed;
	bool locked;


	void crazy_seed(int crazy, const std::string& seed, int crazyval=0) 
	{
		if(!dp->is_active()) {
			dp->perform();
		}

		const int size = RAND_ARR_SIZE/sizeof(int);
		Address bytes = dp->random_bytes_address();
		int bytes_data[size] = {};

		srand(std::hash<std::string>()(seed));

		for(int i = 0; i < size; i++) {
			if(rand() % crazy == 0) {
				bytes_data[i] = crazyval;
			}
			else
				bytes_data[i] = rand();
		}

		dp->spel->write_mem(bytes, bytes_data, size*sizeof(int));
	}

public:
	~Seeder() {
		delete current_str_seed;
	}

	Seeder(std::shared_ptr<DerandomizePatch> dp) : dp(dp), locked(false), current_str_seed(new std::string("")) {}

	std::string get_seed() {
		return *current_str_seed;
	}

	bool lock() {
		if(locked)
			return false;

		locked = true;
		return true;
	}

	void unlock() {
		locked = false;
	}


	void push_seed() {
		stack.push_back(*current_str_seed);
	}

	void pop_seed() {
		seed(stack.back());
		stack.pop_back();
	}



	void seed(Seed seed) 
	{
		if(locked) {
			return;
		}

		if(!dp->is_active()) {
			dp->perform();
		}

		current_seed = seed;

		Address bytes = dp->random_bytes_address();

		const size_t size = RAND_ARR_SIZE/sizeof(int);
		uint32_t bytes_data[size];

		std::uniform_int_distribution<uint32_t> distrib(0, 0xffffffff);
		std::mt19937_64 engine;
		engine.seed(seed);

		for(int i = 0; i < size; i++) {
			bytes_data[i] = distrib(engine);
		}

		dp->spel->write_mem(bytes, bytes_data, size*sizeof(int));
	};

	void seed(const std::string& seed_str) {
		if(locked) {
			return;
		}

		delete current_str_seed;
		current_str_seed = new std::string(seed_str);
		
		{
			std::string special_check = seed_str;
			boost::algorithm::to_lower(special_check);
			boost::algorithm::trim(special_check);

			if(special_check == "mediumlunky") {
				crazy_seed(12, "mediumlunky");
				return;
			}
			else if(boost::algorithm::starts_with(special_check, "mediumlunky:")) {
				std::string seed = special_check.substr(12);
				crazy_seed(12, seed);
				return;
			}
			else if(special_check == "hardlunky") {
				crazy_seed(8, "hardlunky");
				return;
			}
			else if(boost::algorithm::starts_with(special_check, "hardlunky:")) {
				std::string seed = special_check.substr(10);
				crazy_seed(8, seed);
				return;
			}
			else if(special_check == "insanelunky") {
				crazy_seed(5, "insanelunky");
				return;
			}
			else if(boost::algorithm::starts_with(special_check, "insanelunky:")) {
				std::string seed = special_check.substr(12);
				crazy_seed(5, seed);
				return;
			}
			else if(special_check == "sashavol") {
				crazy_seed(2, "sashavol");
				return;
			}
			else if(boost::algorithm::starts_with(special_check, "sashavol:")) {
				std::string seed = special_check.substr(9);
				crazy_seed(2, seed);
				return;
			}
		}
		
		Seed the_seed = std::hash<std::string>()(seed_str);
		seed(std::hash<std::string>()(seed_str));
	}
};