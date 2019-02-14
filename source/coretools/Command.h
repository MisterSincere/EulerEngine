#pragma once

#include <string>
#include <cassert>
#include <vector>


namespace CORETOOLS
{

	struct Cmd {
		/// Constructors for string and cstring
		Cmd(std::string const& cmd) : p_text(cmd.c_str()) {}
		Cmd(char const* cmd) : p_text(cmd) {}

		/// Get the cstring data
		char const* get() const { return p_text; }

		/// Assignment for string and cstring
		Cmd operator=(std::string const& cmd) { p_text = cmd.c_str(); }
		Cmd operator=(char const* cmd) { p_text = cmd; }

		/// Comparison operators for equality amounts
		template<typename T> bool operator==(T equals) const { return equals == p_equals; }
		template<typename T> bool operator>=(T equals) const { return equals >= p_equals; }
		template<typename T> bool operator<=(T equals) const { return equals <= p_equals; }
		template<typename T> bool operator>(T equals) const { return equals > p_equals; }
		template<typename T> bool operator<(T equals) const { return equals < p_equals; }

		/// Equal operators for Command, string and cstring
		bool operator==(Cmd const& cmd) const { return (strcmp(p_text, cmd.get()) == 0); }
		bool operator==(std::string const& cmd) const { return (strcmp(p_text, cmd.c_str()) == 0); }
		bool operator==(char const* cmd) const { return (strcmp(p_text, cmd) == 0); }

		char operator[](unsigned int index) const { return p_text[index]; }

		char const* p_text;
		uint32_t		p_equals{ 0u };
	};

	struct CmdList {
		/// [] operator	for string / cstring comparison and simple index acquiring
		Cmd operator[](unsigned int i) const { return p_cmds[i]; }
		Cmd operator[](std::string const& cmd) const { return operator[](cmd.c_str()); }
		Cmd operator[](char const* cmd) const {
			auto el = std::find(p_cmds.begin(), p_cmds.end(), cmd);
			if (el == p_cmds.end()) assert(false);
			return *el;
		}

		/// += operator to be able to add commands with string / cstring
		CmdList operator+=(char const* cmd) { p_cmds.emplace_back(cmd); return *this; }
		CmdList operator+=(std::string const& cmd) { p_cmds.emplace_back(cmd); return *this; }
		CmdList operator+=(Cmd const& cmd) { p_cmds.push_back(cmd); return *this; }

		/// Wrapper for vector iterators
		auto begin() const { return p_cmds.begin(); }
		auto end() const { return p_cmds.end(); }

		size_t size() const { return p_cmds.size(); }

		std::vector<Cmd> p_cmds;
	};

}