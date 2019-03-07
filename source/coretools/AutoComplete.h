/////////////////////////////////////////////////////////////////////
// Filename: AutoComplete.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <vector>

#include "Command.h"

namespace CORETOOLS
{
	class EEAutoComplete
	{
	public:
		EEAutoComplete(::CORETOOLS::CmdList const&);
		EEAutoComplete(EEAutoComplete const&) = delete;
		EEAutoComplete(EEAutoComplete&&) = delete;

		/**
		 * First string will be changed to the nearest identical string that was set
		 * during creation of this instance
		 *
		 * @param toBeCompleted		The string that we will check against
		 * @param completed				Nearest identical string from the cmdlist
		 *
		 * @return An integer indicating the amount of similiar characters
		 * 
		 * @Deprecated it is recommended to use multicomplete instead
		 **/
		uint32_t Complete(EEstring const& toBeCompleted, EEstring& completed);

		/**
		 * Returns a list of commands that equal that /p toBeCompleted string with at
		 * least /p minimum characters. First elements have most same chars and the
		 * last element the least.
		 *
		 * @param toBeCompleted		The string that this method will check against
		 * @param minimum					The amount of chars the cmd needs to equal at least
		 *
		 * @return List of commands	that are fullfilling the above criteria sorted by amount of equal chars
		 **/
		std::vector<::CORETOOLS::Cmd> MultiComplete(EEstring const& toBeCompleted, uint32_t minimum = 1ui32);

		/**
		 * Allows to set a whole new command list
		 **/
		void SetCommandList(::CORETOOLS::CmdList const&);

		/**
		 * Allows to add a single command to the list
		 **/
		void AddCommandToList(::CORETOOLS::Cmd const&);

		EEAutoComplete& operator=(EEAutoComplete const&) = delete;
		EEAutoComplete& operator=(EEAutoComplete&&) = delete;

	private:
		/**
		 * @return The amount of same characters until the first different character
		 **/
		uint32_t GetEquals(EEcstr string1, EEcstr string2);

	private:
		CmdList m_availableCmds;
	};
}