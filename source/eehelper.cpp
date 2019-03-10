/////////////////////////////////////////////////////////////////////
// Filename: eehelper.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "eehelper.h"

#include <fstream>
#if defined(_WINDOWS)
#include <Windows.h>
#endif
#include <cassert>

std::vector<char> EE::tools::readFile(char const* fileName)
{
	std::ifstream file(fileName, std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		size_t size = static_cast<size_t>(file.tellg());
		std::vector<char> data(size);
		file.seekg(0);
		file.read(data.data(), size);
		file.close();
		return data;
	} else {
		EE_PRINTA("[EETOOLS] Failed to open file %s!\n", fileName);
		exitFatal("[EETOOLS] Failed to open file!");
		return std::vector<char>();
	}
}

std::vector<EEcstr> EE::tools::explodeString(EEcstr str, EEcstr del)
{
	std::vector<EEcstr> res;
	uint32_t l{ 0u };
	for (size_t i = 0u; i <= EE_STRLEN(str); i++) {
		if (i == EE_STRLEN(str) || EE_STRNCMP(&str[i], del, EE_STRLEN(del)) == 0) {
			EEstr cur = new EEchar[l + 1];
			assert(!memcpy_s(cur, sizeof(EEchar) * l, &str[i - l], sizeof(EEchar) * l));
			cur[l] = STR('\0');
			res.push_back(cur);
			i += EE_STRLEN(del) - 1;
			l = 0u;
		} else {
			l++;
		}
	}
	return res;
}

std::vector<EEstring> EE::tools::explodeString(EEstring str, EEstring del)
{
	std::vector<EEstring> res;
	size_t off{ 0u }, pre;
	while (off < str.size()) {
		pre = off;
		off = str.find(del, off);
		auto end = (off == std::string::npos) ? str.end() : str.begin() + off;
		res.emplace_back(str.begin() + pre, end);
		if (off == std::string::npos) break;
		else off++;
	}
	return res;
}

void EE::tools::exitFatal(char const* msg)
{
#if defined(_WINDOWS)
	MessageBoxA(nullptr, msg, "Fatal Error", MB_OK | MB_ICONERROR);
#endif
	assert(false);
}

void EE::tools::warning(char const* msg)
{
#if defined(_WINDOWS)
	MessageBoxA(nullptr, msg, "Warning", MB_OK | MB_ICONERROR);
#endif
}