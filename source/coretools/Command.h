/////////////////////////////////////////////////////////////////////
// Filename: Command.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <cassert>
#include <vector>

#include "eehelper.h"


namespace CORETOOLS
{

	struct SpecificCmd {
		SpecificCmd(EEcstr cmd) : raw(cmd) /* TODO may be unclean*/ {
			std::vector<EEcstr> exp = ::EE::tools::explodeString(cmd);
			this->cmd = exp[0];
			params.insert(params.begin(), exp.begin() + 1, exp.end());
		}
		SpecificCmd(EEstring const& cmd) : SpecificCmd(cmd.c_str()) {}

		/// Comparison for the main command string
		bool operator==(EEcstr cmd)						const { return (EE_STRCMP(cmd, this->cmd)					== 0); }
		bool operator==(EEstring const& cmd)	const { return (EE_STRCMP(cmd.c_str(), this->cmd) == 0); }

		/// Checks if the cmd is the i-th param
		bool operator()(EEcstr cmd, uint32_t i)	const {
			return (i < params.size()) && (EE_STRCMP(cmd, params[i]) == 0);
		}
		bool operator()(EEstring const& cmd, uint32_t i) const { return operator()(cmd.c_str(), i); }

		/// Checks if the param is defined in this command
		bool operator()(EEcstr cmd) const {
			for (const auto& cur : params) if (EE_STRCMP(cur, cmd) == 0) return true;
			return false;
		}
		bool operator()(EEstring const& cmd) const { return operator()(cmd.c_str()); }

		/// Returns the i-th param
		EEcstr operator[](uint32_t i) const { return params[i]; }

		EEcstr							raw;
		EEcstr							cmd;
		std::vector<EEcstr> params;
	};

	struct Cmd {
		/// Constructors for string and cstring
		Cmd(EEstring const& cmd) : raw(cmd.c_str()) {}
		Cmd(EEcstr cmd)					 : raw(cmd) {}

		/// Assignment for string and cstring
		Cmd operator=(EEstring const& cmd) { raw = cmd.c_str(); }
		Cmd operator=(EEcstr cmd)					 { raw = cmd; }

		/// Comparison operators for equality amounts
		template<typename T> bool operator==(T equals) const { return equals == this->equals; }
		template<typename T> bool operator>=(T equals) const { return equals <= this->equals; }
		template<typename T> bool operator<=(T equals) const { return equals >= this->equals; }
		template<typename T> bool operator>(T equals)  const { return equals < this->equals; }
		template<typename T> bool operator<(T equals)  const { return equals > this->equals; }

		/// Equal operators for Command, string and cstring
		bool operator==(Cmd const& cmd)			 const { return (EE_STRCMP(raw, cmd.raw)			== 0); }
		bool operator==(EEstring const& cmd) const { return (EE_STRCMP(raw, cmd.c_str())	== 0); }
		bool operator==(EEcstr cmd)					 const { return (EE_STRCMP(raw, cmd)					== 0); }
		
		EEcstr								raw;
		uint32_t							equals{ 0u };
	};

	struct CmdList {
		/// Creation with a list of commands
		CmdList operator()(std::vector<Cmd> cmds) { this->cmds = cmds; return *this; }
		CmdList operator=(std::vector<Cmd> cmds) { this->cmds = cmds; return *this; }

		/// [] operator	for string / cstring comparison and simple index acquiring
		Cmd operator[](EEcstr cmd) const {
			for (const auto& curCmd : this->cmds) if (curCmd == cmd) return curCmd;
			assert(false);
		}
		Cmd operator[](EEstring const& cmd) const { return operator[](cmd.c_str()); }
		Cmd operator[](unsigned int i)			const { return cmds[i]; }

		/// += operator to be able to add commands with string / cstring
		CmdList operator+=(EEcstr cmd) { cmds.emplace_back(cmd); return *this; }
		CmdList operator+=(EEstring const& cmd) { cmds.emplace_back(cmd); return *this; }
		CmdList operator+=(Cmd const& cmd) { cmds.push_back(cmd); return *this; }

		/// -= operator to be able to remove commands identified by string/cstring/cmd
		CmdList operator-=(EEcstr cmd) {
			auto el = std::find(cmds.begin(), cmds.end(), cmd);
			if (el != cmds.end()) cmds.erase(el);
			else EE_PRINT("[CMDLIST] Element %s not found to be removed!\n", cmd);
			return *this;
		}
		CmdList operator-=(EEstring const& cmd) { return operator-=(cmd.c_str()); }
		CmdList operator-=(Cmd const& cmd)			{ return operator-=(cmd.raw); }

		/// Checks if this cmdlist contains the main command of a specific command
		bool operator>(SpecificCmd const& cmd) const {
			for (auto const& curCmd : this->cmds) if (curCmd == cmd.cmd) return true;
			return false;
		}
		bool operator<(SpecificCmd const& cmd) const { return !operator>(cmd); }

		/// Wrapper for vector iterators and helpmethods
		auto begin() const { return cmds.begin(); }
		auto end()	 const { return cmds.end(); }
		size_t size() const { return cmds.size(); }
		bool empty()	const { return cmds.empty(); }

		std::vector<Cmd> cmds;
	};

}