#pragma once

#include <string>
#include <Windows.h>
#include <WinReg.h>

namespace Registry {
	std::string GetValue(const std::string& key_name);
	void SetValue(const std::string& key_name, const std::string& value);
}