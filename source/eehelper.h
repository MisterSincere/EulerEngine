/////////////////////////////////////////////////////////////////////
// Filename: eehelper.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <vector>

#include "eedefs.h"

namespace EE {
	namespace tools {

		extern std::vector<char> readFile(char const* fileName);

		extern std::vector<EEcstr> explodeString(EEcstr str, EEcstr del = STR(" "));
		extern std::vector<EEstring> explodeString(EEstring str, EEstring del = STR(" "));

		extern void exitFatal(char const* msg);
		extern void warning(char const* msg);



	}
}