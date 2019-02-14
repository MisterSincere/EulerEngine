/////////////////////////////////////////////////////////////////////
// Filename: AutoComplete.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "AutoComplete.h"

#include <algorithm>

#include "Command.h"


CORETOOLS::EEAutoComplete::EEAutoComplete(::CORETOOLS::CmdList const& cmds)
	: m_availableCmds(CmdList(cmds))
{}

uint32_t CORETOOLS::EEAutoComplete::Complete(std::string const& toBeCompleted, std::string& completed)
{
	// Holds the result after all iterations
	uint32_t maxAttachedEquals{ 0u };
	std::string completedString{ toBeCompleted };
	uint32_t curAttachedEquals;

	for (Cmd const& curCmd : m_availableCmds) {
		curAttachedEquals = GetEquals(curCmd.get(), toBeCompleted.c_str());

		// Result check, if new best one and if it is not the same (only new commands will be used)
		if(curAttachedEquals > maxAttachedEquals
			&& strcmp(curCmd.get(), toBeCompleted.c_str())) {
			completedString = curCmd.get();
			maxAttachedEquals = curAttachedEquals;
		}
	}

	// Change reference to detected nearest string
	completed = completedString;

	return maxAttachedEquals;
}

struct greater {
	template<class T>
	bool operator()(T const& a, T const& b) const { return a > b; }
};

std::vector<CORETOOLS::Cmd> CORETOOLS::EEAutoComplete::MultiComplete(std::string const& toBeCompleted, uint32_t minimum)
{
	std::vector<Cmd> equalityMap;

	for (Cmd curCmd : m_availableCmds) {
		curCmd.p_equals = GetEquals(curCmd.get(), toBeCompleted.c_str());

		// Check if it fullfills minimum
		if (curCmd.p_equals >= minimum) equalityMap.push_back(curCmd);
	}

	// Sort the map in descending order of the equals amount
	std::sort(equalityMap.begin(), equalityMap.end(), greater());

	return equalityMap;
}

uint32_t CORETOOLS::EEAutoComplete::GetEquals(char const* string1, char const* string2)
{
	uint32_t equals{ 0u };
	for (size_t i = 0u; i < strlen(string1) && i < strlen(string2); i++) {
		if (string1[i] == string2[i]) equals++;
		else break;
	}
	return equals;
}
