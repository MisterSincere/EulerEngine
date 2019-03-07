/////////////////////////////////////////////////////////////////////
// Filename: Command.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <cassert>
#include <vector>

#include "eedefs.h"


namespace CORETOOLS
{

	struct Cmd {
		/// Constructors for string and cstring
		Cmd(EEstring const& cmd) : p_text(cmd.c_str()) {}
		Cmd(EEcstr cmd)					 : p_text(cmd) {}

		/// Get the cstring data
		EEcstr get() const { return p_text; }

		/// Assignment for string and cstring
		Cmd operator=(EEstring const& cmd) { p_text = cmd.c_str(); }
		Cmd operator=(EEcstr cmd)					 { p_text = cmd; }

		/// Comparison operators for equality amounts
		template<typename T> bool operator==(T equals) const { return equals == p_equals; }
		template<typename T> bool operator>=(T equals) const { return equals >= p_equals; }
		template<typename T> bool operator<=(T equals) const { return equals <= p_equals; }
		template<typename T> bool operator>(T equals)  const { return equals > p_equals; }
		template<typename T> bool operator<(T equals)  const { return equals < p_equals; }

		/// Equal operators for Command, string and cstring
		bool operator==(Cmd const& cmd)			 const { return (EE_STRCMP(p_text, cmd.get())		== 0); }
		bool operator==(EEstring const& cmd) const { return (EE_STRCMP(p_text, cmd.c_str()) == 0); }
		bool operator==(EEcstr cmd)					 const { return (EE_STRCMP(p_text, cmd)					== 0); }

		EEchar operator[](unsigned int index) const { return p_text[index]; }

		EEcstr		p_text;
		uint32_t	p_equals{ 0u };
	};

	struct CmdList {
		/// [] operator	for string / cstring comparison and simple index acquiring
		Cmd operator[](unsigned int i) const { return p_cmds[i]; }
		Cmd operator[](EEstring const& cmd) const { return operator[](cmd.c_str()); }
		Cmd operator[](EEcstr cmd) const {
			auto el = std::find(p_cmds.begin(), p_cmds.end(), cmd);
			if (el == p_cmds.end()) EE_PRINT("[CMDLIST] Element %s not found!\n", cmd);
			return *el;
		}

		/// += operator to be able to add commands with string / cstring
		CmdList operator+=(EEcstr cmd) { p_cmds.emplace_back(cmd); return *this; }
		CmdList operator+=(EEstring const& cmd) { p_cmds.emplace_back(cmd); return *this; }
		CmdList operator+=(Cmd const& cmd) { p_cmds.push_back(cmd); return *this; }

		/// -= operator to be able to remove commands identified by string/cstring/cmd
		CmdList operator-=(EEcstr cmd) {
			auto el = std::find(p_cmds.begin(), p_cmds.end(), cmd);
			if (el != p_cmds.end()) p_cmds.erase(el);
			else EE_PRINT("[CMDLIST] Element %s not found to be removed!\n", cmd);
			return *this;
		}
		CmdList operator-=(EEstring const& cmd) { return operator-=(cmd.c_str()); }
		CmdList operator-=(Cmd const& cmd)			{ return operator-=(cmd.get()); }

		/// Wrapper for vector iterators
		auto begin() const { return p_cmds.begin(); }
		auto end()	 const { return p_cmds.end(); }

		size_t size() const { return p_cmds.size(); }
		bool empty()	const { return p_cmds.empty(); }

		std::vector<Cmd> p_cmds;
	};

}