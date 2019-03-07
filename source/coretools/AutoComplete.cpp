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

uint32_t CORETOOLS::EEAutoComplete::Complete(EEstring const& toBeCompleted, EEstring& completed)
{
	// Holds the result after all iterations
	uint32_t maxAttachedEquals{ 0u };
	EEstring completedString{ toBeCompleted };
	uint32_t curAttachedEquals;

	for (Cmd const& curCmd : m_availableCmds) {
		curAttachedEquals = GetEquals(curCmd.get(), toBeCompleted.c_str());

		// Result check, if new best one and if it is not the same (only new commands will be used)
		if(curAttachedEquals > maxAttachedEquals
			&& EE_STRCMP(curCmd.get(), toBeCompleted.c_str())) {
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

std::vector<CORETOOLS::Cmd> CORETOOLS::EEAutoComplete::MultiComplete(EEstring const& toBeCompleted, uint32_t minimum)
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

void CORETOOLS::EEAutoComplete::SetCommandList(::CORETOOLS::CmdList const& cmds)
{
	m_availableCmds = cmds;
}

void CORETOOLS::EEAutoComplete::AddCommandToList(::CORETOOLS::Cmd const& cmd)
{
	m_availableCmds += cmd;
}

uint32_t CORETOOLS::EEAutoComplete::GetEquals(EEcstr string1, EEcstr string2)
{
	uint32_t equals{ 0u };
	for (size_t i = 0u; i < EE_STRLEN(string1) && i < EE_STRLEN(string2); i++) {
		if (string1[i] == string2[i]) equals++;
		else break;
	}
	return equals;
}
