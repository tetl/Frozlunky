#pragma once

#include <memory>
#include <windows.h>
#include "signature.h"
#include "spelunky.h"
#include "debug.h"
#include <iomanip>
#include <iostream>

class Patch {
private:
	bool active;
	bool is_locked;

	virtual bool _undo() = 0;
	virtual bool _perform() = 0;

public:
	std::shared_ptr<Spelunky> spel;
	
	Patch(std::shared_ptr<Spelunky> spel) : active(false), spel(spel), is_locked(false) {}

	virtual void lock() { is_locked = true; }
	virtual void unlock() { is_locked = false; }

	bool locked() {
		return is_locked;
	}

	void undo() {
		if(is_locked) {
			return;
		}

		if(this->valid() && _undo()) {
			active = false;
		}
	}

	virtual bool valid() = 0;

	void perform() {
		if(is_locked) {
			return;
		}

		if(this->valid() && _perform()) {
			active = true;
		}
	}

	bool is_active() {
		return active;
	}

	virtual ~Patch() {}
};


