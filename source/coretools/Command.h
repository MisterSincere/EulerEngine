/////////////////////////////////////////////////////////////////////
// Filename: Command.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <cassert>
#include <vector>

#include "eedefs.h"


namespace CORETOOLS
{

	struct Cmd {
		/// Constructors for string and cstring
		Cmd(std::wstring const& cmd) : p_text(cmd.c_str()) {}
		Cmd(wchar_t const* cmd) : p_text(cmd) {}

		/// Get the cstring data
		wchar_t const* get() const { return p_text; }

		/// Assignment for string and cstring
		Cmd operator=(std::wstring const& cmd) { p_text = cmd.c_str(); }
		Cmd operator=(wchar_t const* cmd) { p_text = cmd; }

		/// Comparison operators for equality amounts
		template<typename T> bool operator==(T equals) const { return equals == p_equals; }
		template<typename T> bool operator>=(T equals) const { return equals >= p_equals; }
		template<typename T> bool operator<=(T equals) const { return equals <= p_equals; }
		template<typename T> bool operator>(T equals) const { return equals > p_equals; }
		template<typename T> bool operator<(T equals) const { return equals < p_equals; }

		/// Equal operators for Command, string and cstring
		bool operator==(Cmd const& cmd) const { return (wcscmp(p_text, cmd.get()) == 0); }
		bool operator==(std::wstring const& cmd) const { return (wcscmp(p_text, cmd.c_str()) == 0); }
		bool operator==(wchar_t const* cmd) const { return (wcscmp(p_text, cmd) == 0); }

		wchar_t operator[](unsigned int index) const { return p_text[index]; }

		wchar_t const* p_text;
		uint32_t		p_equals{ 0u };
	};

	struct CmdList {
		/// [] operator	for string / cstring comparison and simple index acquiring
		Cmd operator[](unsigned int i) const { return p_cmds[i]; }
		Cmd operator[](std::wstring const& cmd) const { return operator[](cmd.c_str()); }
		Cmd operator[](wchar_t const* cmd) const {
			auto el = std::find(p_cmds.begin(), p_cmds.end(), cmd);
			if (el == p_cmds.end()) assert(false);
			return *el;
		}

		/// += operator to be able to add commands with string / cstring
		CmdList operator+=(wchar_t const* cmd) { p_cmds.emplace_back(cmd); return *this; }
		CmdList operator+=(std::wstring const& cmd) { p_cmds.emplace_back(cmd); return *this; }
		CmdList operator+=(Cmd const& cmd) { p_cmds.push_back(cmd); return *this; }

		/// -= operator to be able to remove commands identified by string/cstring/cmd
		CmdList operator-=(wchar_t const* cmd) {
			auto el = std::find(p_cmds.begin(), p_cmds.end(), cmd);
			if (el != p_cmds.end()) p_cmds.erase(el);
			else EE_PRINT("[CMDLIST] Element %s not found to be removed!\n", cmd);
			return *this;
		}
		CmdList operator-=(std::wstring const& cmd) { return operator-=(cmd.c_str()); }
		CmdList operator-=(Cmd const& cmd) { return operator-=(cmd.get()); }

		/// Wrapper for vector iterators
		auto begin() const { return p_cmds.begin(); }
		auto end() const { return p_cmds.end(); }

		size_t size() const { return p_cmds.size(); }
		bool empty() const { return p_cmds.empty(); }

		std::vector<Cmd> p_cmds;
	};

}